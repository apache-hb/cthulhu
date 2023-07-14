#include "common.h"

#include "std/str.h"
#include "std/map.h"
#include "std/set.h"
#include "std/vector.h"

#include "io/fs.h"

#include "base/memory.h"
#include "base/panic.h"

#include <string.h>

typedef struct c89_source_t {
    io_t *io;
    const char *path;
} c89_source_t;

typedef struct c89_emit_t {
    emit_t emit;

    map_t *modmap; // map<ssa_symbol, ssa_module>

    map_t *srcmap; // map<ssa_module, c89_source>
    map_t *hdrmap; // map<ssa_module, c89_source>

    fs_t *fs;
    map_t *deps;
    vector_t *sources;
} c89_emit_t;

static c89_source_t *source_new(io_t *io, const char *path)
{
    c89_source_t *source = ctu_malloc(sizeof(c89_source_t));
    source->io = io;
    source->path = path;
    return source;
}

static c89_source_t *header_for(c89_emit_t *emit, const ssa_module_t *mod, const char *path)
{
    io_t *io = fs_open(emit->fs, path, eAccessWrite | eAccessText);
    c89_source_t *source = source_new(io, path);
    map_set_ptr(emit->hdrmap, mod, source);
    return source;
}

static c89_source_t *source_for(c89_emit_t *emit, const ssa_module_t *mod, const char *path)
{
    io_t *io = fs_open(emit->fs, path, eAccessWrite | eAccessText);
    c89_source_t *source = source_new(io, path);
    map_set_ptr(emit->srcmap, mod, source);
    return source;
}

// begin api

static bool check_root_mod(vector_t *path, const char *id)
{
    const char *tail = vector_tail(path);
    return str_equal(tail, id);
}

static char *format_path(const char *base, const char *name, const char *dir, const char *ext)
{
    if (strlen(base) == 0) { return format("%s/%s.%s", dir, name, ext); }
    return format("%s/%s/%s.%s", dir, base, name, ext);
}

static void begin_c89_module(c89_emit_t *emit, const ssa_module_t *mod)
{
    fs_t *fs = emit->fs;

    // this is really badly named :p
    // whats really going on is that we're checking if the module has the same name.
    // as the last element in the path, in these cases we dont want to emit the last element of the path.
    // this is not required for correctness but makes the output nicer to grok.
    bool isRootMod = check_root_mod(mod->path, mod->name);

    vector_t *vec = vector_clone(mod->path); // lets not scuff the original path
    if (isRootMod) { vector_drop(vec); }

    char *path = str_join("/", vec);
    char *srcFile = format_path(path, mod->name, "src", "c");
    char *hdrFile = format_path(path, mod->name, "include", "h");

    fs_dir_create(fs, path);
    fs_file_create(fs, srcFile);
    fs_file_create(fs, hdrFile);

    vector_push(&emit->sources, srcFile);

    c89_source_t *src = source_for(emit, mod, srcFile);
    c89_source_t *hdr = header_for(emit, mod, hdrFile);

    write_string(hdr->io, "#pragma once\n");
    write_string(src->io, "#include \"%s\"\n", hdrFile);
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

static const char *get_c89_digit(ssa_type_digit_t ty)
{
    switch (ty.digit)
    {
    case eDigitChar: return (ty.sign == eSignUnsigned) ? "unsigned char" : "char";
    case eDigitShort: return (ty.sign == eSignUnsigned) ? "unsigned short" : "short";
    case eDigitInt: return (ty.sign == eSignUnsigned) ? "unsigned int" : "int";
    case eDigitLong: return (ty.sign == eSignUnsigned) ? "unsigned long" : "long";
    case eDigitSize: return (ty.sign == eSignUnsigned) ? "size_t" : "ptrdiff_t";
    case eDigitPtr: return (ty.sign == eSignUnsigned) ? "uintptr_t" : "intptr_t";

    default: NEVER("unknown digit %d", ty.digit);
    }
}

static const char *get_c89_quals(quals_t quals)
{
    // const is the default
    if (quals == eQualDefault) { return "const "; }

    vector_t *vec = vector_new(3);
    if (quals & eQualAtomic) { vector_push(&vec, "_Atomic"); }
    if (quals & eQualVolatile) { vector_push(&vec, "volatile"); }
    if (quals & ~eQualMutable) { vector_push(&vec, "const"); }

    return str_join(" ", vec);
}

static const char *format_c89_type(c89_emit_t *emit, const ssa_type_t *type, const char *name)
{
    CTASSERT(type != NULL);
    const char *quals = get_c89_quals(type->quals);

    switch (type->kind)
    {
    case eTypeEmpty:
    case eTypeUnit:
        NEVER("cannot emit this type %d", type->kind);

    case eTypeBool: return (name != NULL) ? format("%sbool %s", quals, name) : format("%sbool", quals);
    case eTypeDigit: {
        const char *digitName = get_c89_digit(type->digit);
        return (name != NULL) ? format("%s%s %s", quals, digitName, name) : format("%s%s", quals, digitName);
    }

    default: NEVER("unknown type %d", type->kind);
    }
}

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
    set_t *requires = set_new(len); // set of modules required by this module

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

static void emit_global(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *global)
{
    c89_source_t *src = map_get_ptr(emit->srcmap, mod);
    c89_source_t *hdr = map_get_ptr(emit->hdrmap, mod);

    const char *it = format_c89_type(emit, global->type, global->name);

    const char *link = format_c89_link(global->linkage);

    // TODO: calc value

    switch (global->visibility)
    {
    case eVisiblePublic:
        CTASSERT(global->linkage != eLinkModule); // TODO: boot this check into the checker

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

static void emit_c89_module(c89_emit_t *emit, const ssa_module_t *mod)
{
    emit_required_headers(emit, mod);
    emit_globals(emit, mod);
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
        emit_c89_module(&emit, mod);
    }

    c89_emit_result_t result = {
        .sources = emit.sources,
    };
    return result;
}
