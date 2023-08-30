#include "c89.h"

#include "std/str.h"
#include "std/map.h"
#include "std/set.h"
#include "std/vector.h"
#include "std/typed/vector.h"

#include "report/report.h"

#include "io/fs.h"

#include "base/memory.h"
#include "base/panic.h"

#include <string.h>

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

static void collect_deps(c89_emit_t *emit, const ssa_module_t *mod, vector_t *symbols)
{
    size_t len = vector_len(symbols);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_symbol_t *global = vector_get(symbols, i);
        map_set_ptr(emit->modmap, global, (ssa_module_t*)mod);
    }
}

static void c89_begin_module(c89_emit_t *emit, const ssa_module_t *mod)
{
    // collect all symbols defined in this module
    collect_deps(emit, mod, mod->globals);
    collect_deps(emit, mod, mod->functions);

    // create source and header files
    char *path = begin_module(&emit->emit, emit->fs, mod); // lets not scuff the original path

    const char *srcFile = format_path(path, mod->name);
    const char *hdrFile = format_path(path, mod->name);

    vector_push(&emit->sources, format("src/%s.c", srcFile));

    c89_source_t *src = source_for(emit, mod, srcFile);
    c89_source_t *hdr = header_for(emit, mod, hdrFile);

    write_string(hdr->io, "#pragma once\n");
    write_string(hdr->io, "#include <stdbool.h>\n");
    write_string(hdr->io, "#include <stdint.h>\n");

    write_string(src->io, "#include \"%s.h\"\n", hdrFile);
}

// emit api

static const char *format_c89_link(tree_link_t linkage)
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
    switch (symbol->linkage)
    {
    case eLinkEntryCli: return "main";
    case eLinkEntryGui: return "WinMain";
    default: break;
    }

    if (symbol->linkName != NULL) { return symbol->linkName; }
    return symbol->name;
}

static bool is_entry_point(tree_link_t link)
{
    return link == eLinkEntryCli || link == eLinkEntryGui;
}

static const char *format_symbol(c89_emit_t *emit, const ssa_type_t *type, const char *name)
{
    return c89_format_type(emit, type, name, true);
}

static const char *format_local(c89_emit_t *emit, const ssa_type_t *type, const char *name)
{
    return c89_format_type(emit, type, name, false);
}

static void c89_proto_global(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *global)
{
    c89_source_t *src = map_get_ptr(emit->srcmap, mod);
    c89_source_t *hdr = map_get_ptr(emit->hdrmap, mod);

    const char *it = format_symbol(emit, global->type, mangle_symbol_name(global));

    const char *link = format_c89_link(global->linkage);

    if (global->visibility == eVisiblePublic)
    {
        CTASSERT(global->linkage != eLinkModule); // TODO: move this check into the checker

        write_string(hdr->io, "%s%s[1];\n", link, it);
    }
    else
    {
        write_string(src->io, "%s%s[1];\n", link, it);
    }
}

static void c89_proto_function(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *func)
{
    // dont generate prototypes for entry points
    if (is_entry_point(func->linkage)) { return; }

    c89_source_t *src = map_get_ptr(emit->srcmap, mod);
    c89_source_t *hdr = map_get_ptr(emit->hdrmap, mod);

    const ssa_type_t *type = func->type;

    CTASSERTF(type->kind == eTypeClosure, "expected closure type on %s, got %d", func->name, type->kind);

    ssa_type_closure_t closure = type->closure;
    const char *params = c89_format_params(emit, closure.params, closure.variadic);
    const char *result = format_symbol(emit, closure.result, mangle_symbol_name(func));

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
}

static void proto_symbols(c89_emit_t *emit, const ssa_module_t *mod, vector_t *vec, void (*fn)(c89_emit_t*, const ssa_module_t*, const ssa_symbol_t*))
{
    size_t len = vector_len(vec);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_symbol_t *symbol = vector_get(vec, i);
        fn(emit, mod, symbol);
    }
}

static void c89_proto_module(c89_emit_t *emit, const ssa_module_t *mod)
{
    emit_required_headers(emit, mod);
    proto_symbols(emit, mod, mod->globals, c89_proto_global);
    proto_symbols(emit, mod, mod->functions, c89_proto_function);
}

/// bbs

static const ssa_type_t *get_operand_type(c89_emit_t *emit, ssa_operand_t operand)
{
    switch (operand.kind)
    {
    case eOperandImm: {
        const ssa_value_t *value = operand.value;
        return value->type;
    }
    case eOperandLocal: {
        const ssa_local_t *local = typevec_offset(emit->current->locals, operand.local);
        return local->type;
    }
    case eOperandParam: {
        const ssa_local_t *param = typevec_offset(emit->current->params, operand.param);
        return param->type;
    }
    case eOperandGlobal: {
        const ssa_symbol_t *global = operand.global;
        return global->type;
    }
    case eOperandFunction: {
        const ssa_symbol_t *func = operand.function;
        return func->type;
    }
    case eOperandReg: {
        const ssa_block_t *bb = operand.vregContext;
        const ssa_step_t *step = typevec_offset(bb->steps, operand.vregIndex);
        const ssa_type_t *type = map_get_ptr(emit->stepmap, step);
        return type;
    }

    default: NEVER("unknown operand kind %d", operand.kind);
    }
}

static void set_step_type(c89_emit_t *emit, const ssa_step_t *step, const ssa_type_t *type)
{
    map_set_ptr(emit->stepmap, step, (ssa_type_t*)type);
}

static const char *c89_name_vreg(c89_emit_t *emit, const ssa_step_t *step, const ssa_type_t *type)
{
    set_step_type(emit, step, (ssa_type_t*)type);

    const char *id = format("vreg%s", get_step_name(&emit->emit, step));
    return format_symbol(emit, type, id);
}

static const char *c89_name_vreg_by_operand(c89_emit_t *emit, const ssa_step_t *step, ssa_operand_t operand)
{
    const ssa_type_t *type = get_operand_type(emit, operand);
    return c89_name_vreg(emit, step, type);
}

static const char *c89_format_value(c89_emit_t *emit, const ssa_value_t* value)
{
    const ssa_type_t *type = value->type;
    switch (type->kind)
    {
    case eTypeBool: return value->boolValue ? "true" : "false";
    case eTypeDigit: return mpz_get_str(NULL, 10, value->digitValue);
    case eTypeString: return format("\"%s\"", str_normalizen(value->stringValue, value->stringLength));
    default: NEVER("unknown type kind %d", type->kind);
    }
}

static const char *c89_format_local(c89_emit_t *emit, size_t local)
{
    typevec_t *locals = emit->current->locals;
    if (local >= typevec_len(locals))
    {
        report(emit->emit.reports, eFatal, NULL, "local(%zu) > locals(%zu)", local, typevec_len(locals));
        return format("local[error(%zu > %zu)]", local, typevec_len(locals));
    }

    const ssa_local_t *it = typevec_offset(locals, local);
    return it->name;
}

static const char *c89_format_param(c89_emit_t *emit, size_t param)
{
    typevec_t *params = emit->current->params;
    if (param >= typevec_len(params))
    {
        report(emit->emit.reports, eFatal, NULL, "param(%zu) > params(%zu)", param, typevec_len(params));
        return format("param[error(%zu > %zu)]", param, typevec_len(params));
    }

    const ssa_local_t *it = typevec_offset(params, param);
    return format("(&%s)", it->name);
}

static const char *c89_format_operand(c89_emit_t *emit, ssa_operand_t operand)
{
    switch (operand.kind)
    {
    case eOperandEmpty: return "/* empty */";
    case eOperandImm:
        return c89_format_value(emit, operand.value);

    case eOperandBlock:
        return format("bb%s", get_block_name(&emit->emit, operand.bb));

    case eOperandReg:
        return format("vreg%s", get_step_from_block(&emit->emit, operand.vregContext, operand.vregIndex));

    case eOperandGlobal:
        return mangle_symbol_name(operand.global);

    case eOperandFunction:
        return mangle_symbol_name(operand.function);

    case eOperandLocal:
        return c89_format_local(emit, operand.local);

    case eOperandParam:
        return c89_format_param(emit, operand.param);

    default: NEVER("unknown operand kind %d", operand.kind);
    }
}

static bool operand_is_empty(ssa_operand_t operand)
{
    return operand.kind == eOperandEmpty;
}

static bool operand_cant_return(ssa_operand_t operand)
{
    if (operand.kind == eOperandImm)
    {
        const ssa_value_t *value = operand.value;
        const ssa_type_t *type = value->type;
        return type->kind == eTypeUnit || type->kind == eTypeEmpty;
    }

    return operand.kind == eOperandEmpty;
}

static void c89_write_block(c89_emit_t *emit, io_t *io, const ssa_block_t *bb)
{
    size_t len = typevec_len(bb->steps);
    write_string(io, "bb%s: /* len = %zu */\n", get_block_name(&emit->emit, bb), len);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_step_t *step = typevec_offset(bb->steps, i);
        switch (step->opcode)
        {
        case eOpStore: {
            ssa_store_t store = step->store;
            write_string(io, "\t%s[0] = %s;\n", c89_format_operand(emit, store.dst), c89_format_operand(emit, store.src));
            break;
        }
        case eOpLoad: {
            ssa_load_t load = step->load;
            write_string(io, "\t%s = %s[0];\n",
                c89_name_vreg_by_operand(emit, step, load.src),
                c89_format_operand(emit, load.src)
            );
            break;
        }
        case eOpUnary: {
            ssa_unary_t unary = step->unary;
            write_string(io, "\t%s = (%s %s);\n",
                c89_name_vreg_by_operand(emit, step, unary.operand),
                unary_symbol(unary.unary),
                c89_format_operand(emit, unary.operand)
            );
            break;
        }
        case eOpBinary: {
            ssa_binary_t bin = step->binary;
            write_string(io, "\t%s = (%s %s %s);\n",
                c89_name_vreg_by_operand(emit, step, bin.lhs),
                c89_format_operand(emit, bin.lhs),
                binary_symbol(bin.binary),
                c89_format_operand(emit, bin.rhs)
            );
            break;
        }
        case eOpCompare: {
            ssa_compare_t cmp = step->compare;
            write_string(io, "\t%s = (%s %s %s);\n",
                c89_name_vreg(emit, step, ssa_type_bool("bool", eQualDefault)),
                c89_format_operand(emit, cmp.lhs),
                compare_symbol(cmp.compare),
                c89_format_operand(emit, cmp.rhs)
            );
            break;
        }

        case eOpCall: {
            ssa_call_t call = step->call;
            size_t len = typevec_len(call.args);

            const ssa_type_t *ty = get_operand_type(emit, call.function);
            ssa_type_closure_t closure = ty->closure;
            const ssa_type_t *result = closure.result;

            vector_t *args = vector_of(len);
            for (size_t i = 0; i < len; i++)
            {
                const ssa_operand_t *operand = typevec_offset(call.args, i);
                vector_set(args, i, (char*)c89_format_operand(emit, *operand));
            }

            write_string(io, "\t");

            if (result->kind != eTypeEmpty && result->kind != eTypeUnit)
            {
                write_string(io, "%s = ", c89_name_vreg(emit, step, result));
            }

            write_string(io, "%s(%s);\n",
                c89_format_operand(emit, call.function),
                str_join(", ", args)
            );
            break;
        }

        case eOpJump: {
            ssa_jump_t jmp = step->jump;
            write_string(io, "\tgoto %s;\n", c89_format_operand(emit, jmp.target));
            break;
        }
        case eOpBranch: {
            ssa_branch_t br = step->branch;
            write_string(io, "\tif (%s) { goto %s; }", c89_format_operand(emit, br.cond), c89_format_operand(emit, br.then));
            if (!operand_is_empty(br.other))
            {
                write_string(io, " else { goto %s; }", c89_format_operand(emit, br.other));
            }
            write_string(io, "\n");
            break;
        }
        case eOpReturn: {
            ssa_return_t ret = step->ret;
            if (!operand_cant_return(ret.value))
            {
                write_string(io, "\treturn %s;\n", c89_format_operand(emit, ret.value));
            }
            else
            {
                write_string(io, "\treturn;\n");
            }
            break;
        }

        default: NEVER("unknown opcode %d", step->opcode);
        }
    }
}

/// defines

void c89_define_global(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *symbol)
{
    c89_source_t *src = map_get_ptr(emit->srcmap, mod);

    const char *it = format_symbol(emit, symbol->type, mangle_symbol_name(symbol));
    const char *link = format_c89_link(symbol->linkage);

    if (symbol->linkage != eLinkImport)
    {
        const ssa_value_t *value = symbol->value;
        if (value->init)
        {
            const char *valStr = c89_format_value(emit, value);
            write_string(src->io, "%s%s[1] = { %s };\n", link, it, valStr);
        }
        else
        {
            write_string(src->io, "%s%s[1];\n", link, it); // TODO: is this needed
        }
    }
}

static void write_locals(c89_emit_t *emit, io_t *io, typevec_t *locals)
{
    size_t len = typevec_len(locals);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_local_t *local = typevec_offset(locals, i);
        write_string(io, "\t%s[1];\n",
            format_local(emit, local->type, local->name)
        );
    }
}

void c89_define_function(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *func)
{
    c89_source_t *src = map_get_ptr(emit->srcmap, mod);

    const ssa_type_t *type = func->type;
    CTASSERTF(type->kind == eTypeClosure, "expected closure type on %s, got %d", func->name, type->kind);

    ssa_type_closure_t closure = type->closure;
    const char *params = c89_format_params(emit, closure.params, closure.variadic);
    const char *result = format_symbol(emit, closure.result, mangle_symbol_name(func));

    const char *link = format_c89_link(func->linkage);

    if (func->linkage != eLinkImport)
    {
        write_string(src->io, "%s%s(%s) {\n", link, result, params);
        write_locals(emit, src->io, func->locals);
        write_string(src->io, "\tgoto bb%s;\n", get_block_name(&emit->emit, func->entry));
        size_t len = vector_len(func->blocks);
        for (size_t i = 0; i < len; i++)
        {
            const ssa_block_t *bb = vector_get(func->blocks, i);
            c89_write_block(emit, src->io, bb);
        }
        write_string(src->io, "}\n");
    }
}

static void define_symbols(c89_emit_t *emit, const ssa_module_t *mod, vector_t *vec, void (*fn)(c89_emit_t*, const ssa_module_t*, const ssa_symbol_t*))
{
    size_t len = vector_len(vec);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_symbol_t *symbol = vector_get(vec, i);
        emit->current = symbol;
        fn(emit, mod, symbol);
    }
}

static void c89_define_module(c89_emit_t *emit, const ssa_module_t *mod)
{
    define_symbols(emit, mod, mod->globals, c89_define_global);
    define_symbols(emit, mod, mod->functions, c89_define_function);
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

        .stepmap = map_optimal(64),

        .fs = opts.fs,
        .deps = opts.deps,
        .sources = vector_new(32)
    };

    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(opts.modules, i);
        c89_begin_module(&emit, mod);
    }

    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(opts.modules, i);
        c89_proto_module(&emit, mod);
    }

    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(opts.modules, i);
        c89_define_module(&emit, mod);
    }

    c89_emit_result_t result = {
        .sources = emit.sources,
    };
    return result;
}
