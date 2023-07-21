#include "c89.h"

#include "std/str.h"
#include "std/map.h"
#include "std/set.h"
#include "std/vector.h"
#include "std/typed/vector.h"

#include "io/fs.h"

#include "base/memory.h"
#include "base/panic.h"

#include <string.h>

typedef struct c89_source_t {
    io_t *io;
    const char *path;
} c89_source_t;

static c89_source_t *source_new(io_t *io, const char *path)
{
    c89_source_t *source = ctu_malloc(sizeof(c89_source_t));
    source->io = io;
    source->path = path;
    return source;
}

static c89_source_t *header_for(c89_emit_t *emit, const ssa_module_t *mod, const char *path)
{
    char *it = format("include/%s.h", path);
    fs_file_create(emit->fs, it);

    io_t *io = fs_open(emit->fs, it, eAccessWrite | eAccessText);
    c89_source_t *source = source_new(io, format("%s.h", path));
    map_set_ptr(emit->hdrmap, mod, source);
    return source;
}

static c89_source_t *source_for(c89_emit_t *emit, const ssa_module_t *mod, const char *path)
{
    char *it = format("src/%s.c", path);
    fs_file_create(emit->fs, it);

    io_t *io = fs_open(emit->fs, it, eAccessWrite | eAccessText);
    c89_source_t *source = source_new(io, it);
    map_set_ptr(emit->srcmap, mod, source);
    return source;
}

// begin api

static const char *format_path(const char *base, const char *name)
{
    if (strlen(base) == 0) { return name; }
    return format("%s/%s", base, name);
}

static void begin_c89_module(c89_emit_t *emit, const ssa_module_t *mod)
{
    char *path = begin_module(&emit->emit, emit->fs, mod); // lets not scuff the original path

    const char *srcFile = format_path(path, mod->name);
    const char *hdrFile = format_path(path, mod->name);

    vector_push(&emit->sources, format("src/%s.c", srcFile));

    c89_source_t *src = source_for(emit, mod, srcFile);
    c89_source_t *hdr = header_for(emit, mod, hdrFile);

    write_string(hdr->io, "#pragma once\n");
    write_string(src->io, "#include \"%s.h\"\n", hdrFile);
}

// collect api

static void collect_deps(c89_emit_t *emit, const ssa_module_t *mod, vector_t *symbols)
{
    size_t len = vector_len(symbols);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_symbol_t *global = vector_get(symbols, i);
        map_set_ptr(emit->modmap, global, (ssa_module_t*)mod);
    }
}

static void collect_c89_symbols(c89_emit_t *emit, const ssa_module_t *mod)
{
    collect_deps(emit, mod, mod->globals);
    collect_deps(emit, mod, mod->functions);
}

// emit api

static const char *format_c89_link(h2_link_t linkage)
{
    switch (linkage)
    {
    case eLinkImport: return "extern ";
    case eLinkExport: return "";
    case eLinkModule: return "static ";

    default: return ""; // TODO: hmmmm
    }
}

static void get_required_headers(c89_emit_t *emit, set_t *requires, const ssa_module_t *root, vector_t *symbols)
{
    size_t len = vector_len(symbols);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_symbol_t *global = vector_get(symbols, i);
        set_t *deps = map_get_ptr(emit->deps, global);
        if (deps == NULL) { continue; }

        set_iter_t iter = set_iter(deps);
        while (set_has_next(&iter))
        {
            const ssa_symbol_t *dep = set_next(&iter);
            const ssa_module_t *depMod = map_get_ptr(emit->modmap, dep);
            if (depMod != root)
            {
                set_add_ptr(requires, depMod);
            }
        }
    }
}

static void emit_required_headers(c89_emit_t *emit, const ssa_module_t *mod)
{
    // TODO: this is very coarse, we should only add deps to the headers
    // for symbols that are externally visible

    size_t len = vector_len(mod->globals);
    set_t *requires = set_new(MAX(len, 1)); // set of modules required by this module

    get_required_headers(emit, requires, mod, mod->globals);
    get_required_headers(emit, requires, mod, mod->functions);

    c89_source_t *header = map_get_ptr(emit->hdrmap, mod);
    set_iter_t iter = set_iter(requires);
    while (set_has_next(&iter))
    {
        const ssa_module_t *item = set_next(&iter);
        c89_source_t *dep = map_get_ptr(emit->hdrmap, item);
        write_string(header->io, "#include \"%s\"\n", dep->path);
    }
}

static const char *mangle_symbol_name(const ssa_symbol_t *symbol)
{
    if (symbol->linkName != NULL) { return symbol->linkName; }
    return symbol->name;
}

static void emit_global(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *global)
{
    c89_source_t *src = map_get_ptr(emit->srcmap, mod);
    c89_source_t *hdr = map_get_ptr(emit->hdrmap, mod);

    const char *it = c89_format_type(emit, global->type, mangle_symbol_name(global));

    const char *link = format_c89_link(global->linkage);

    // TODO: calc value

    switch (global->visibility)
    {
    case eVisiblePublic:
        CTASSERT(global->linkage != eLinkModule); // TODO: move this check into the checker

        write_string(hdr->io, "%s%s;\n", link, it);
        write_string(src->io, "%s%s;\n", link, it);
        break;
    case eVisiblePrivate:
        write_string(src->io, "%s%s;\n", link, it);
        break;

    default: NEVER("unknown visibility %d", global->visibility);
    }
}

static void emit_globals(c89_emit_t *emit, const ssa_module_t *mod)
{
    size_t len = vector_len(mod->globals);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_symbol_t *global = vector_get(mod->globals, i);
        emit_global(emit, mod, global);
    }
}

static void emit_function(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *func)
{
    c89_source_t *src = map_get_ptr(emit->srcmap, mod);
    c89_source_t *hdr = map_get_ptr(emit->hdrmap, mod);

    const ssa_type_t *type = func->type;

    CTASSERTF(type->kind == eTypeClosure, "expected closure type on %s, got %d", func->name, type->kind);

    ssa_type_closure_t closure = type->closure;
    const char *params = c89_format_params(emit, closure.params, closure.variadic);
    const char *result = c89_format_type(emit, closure.result, mangle_symbol_name(func));

    const char *link = format_c89_link(func->linkage);

    if (func->visibility == eVisiblePublic)
    {
        CTASSERT(func->linkage != eLinkModule);
        write_string(hdr->io, "%s%s(%s);\n", link, result, params);
    }
    else
    {
        write_string(src->io, "%s%s(%s);\n", link, result, params);
    }

    if (func->entry != NULL)
    {
        write_string(src->io, "%s%s(%s) {\n", link, result, params);

        write_string(src->io, "}\n");
    }

    switch (func->visibility)
    {
    case eVisiblePublic:
        CTASSERT(func->linkage != eLinkModule); // TODO: move this check into the checker

        write_string(hdr->io, "%s%s(%s);\n", link, result, params);
        write_string(src->io, "%s%s(%s);\n", link, result, params);
        break;
    case eVisiblePrivate:
        write_string(src->io, "%s%s(%s);\n", link, result, params);
        break;

    default: NEVER("unknown visibility %d", func->visibility);
    }
}

static void emit_functions(c89_emit_t *emit, const ssa_module_t *mod)
{
    size_t len = vector_len(mod->functions);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_symbol_t *func = vector_get(mod->functions, i);
        emit_function(emit, mod, func);
    }
}

static void emit_c89_module(c89_emit_t *emit, const ssa_module_t *mod)
{
    emit_required_headers(emit, mod);
    emit_globals(emit, mod);
    emit_functions(emit, mod);
}

static void fwd_c89_module(c89_emit_t *emit, const ssa_module_t *mod)
{

}

c89_emit_result_t emit_c89(const c89_emit_options_t *options)
{
    emit_options_t opts = options->opts;
    size_t len = vector_len(opts.modules);

    c89_emit_t emit = {
        .emit = {
            .reports = opts.reports,
            .blockNames = names_new(64),
            .vregNames = names_new(64),
        },
        .modmap = map_optimal(len),
        .srcmap = map_optimal(len),
        .hdrmap = map_optimal(len),

        .fs = opts.fs,
        .deps = opts.deps,
        .sources = vector_new(32)
    };

    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(opts.modules, i);
        collect_c89_symbols(&emit, mod);
    }

    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(opts.modules, i);
        begin_c89_module(&emit, mod);
    }

    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(opts.modules, i);
        fwd_c89_module(&emit, mod);
    }

    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(opts.modules, i);
        emit_c89_module(&emit, mod);
    }

    c89_emit_result_t result = {
        .sources = emit.sources,
    };
    return result;
}
