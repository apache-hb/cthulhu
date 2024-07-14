// SPDX-License-Identifier: LGPL-3.0-only

#include "cthulhu/broker/broker.h"

#include "base/util.h"
#include "c89.h"

#include "io/io.h"
#include "notify/notify.h"
#include "std/str.h"
#include "std/map.h"
#include "std/set.h"
#include "std/vector.h"
#include "std/typed/vector.h"

#include "os/os.h"

#include "fs/fs.h"

#include "arena/arena.h"

#include "base/panic.h"
#include "core/macros.h"

#include <limits.h>
#include <stdio.h>

static int integer_fits_longlong(const mpz_t value)
{
    // mini gmp doesnt have functions that take longlong so this is a bit of a hack
    mpz_t max;
    mpz_init_set_si(max, 2);
    mpz_pow_ui(max, max, sizeof(long long) * CHAR_BIT - 1);
    mpz_sub_ui(max, max, 1);

    mpz_t min;
    mpz_init_set_si(min, 2);
    mpz_pow_ui(min, min, sizeof(long long) * CHAR_BIT - 1);

    mpz_neg(min, min);

    return mpz_cmp(value, max) <= 0 && mpz_cmp(value, min) >= 0;
}

static int integer_is_unsigned(const mpz_t value)
{
    return mpz_sgn(value) >= 0;
}

static c89_source_t *source_new(io_t *io, const char *path, arena_t *arena)
{
    c89_source_t *source = ARENA_MALLOC(sizeof(c89_source_t), path, io, arena);
    source->io = io;
    source->path = path;
    return source;
}

static c89_source_t *header_for(c89_emit_t *emit, const ssa_module_t *mod, const char *path)
{
    char *it = str_format(emit->arena, "include/%s.h", path);
    fs_file_create(emit->fs, it);

    io_t *io = fs_open(emit->fs, it, eOsAccessWrite | eOsAccessTruncate);
    c89_source_t *source = source_new(io, str_format(emit->arena, "%s.h", path), emit->arena);
    map_set(emit->hdrmap, mod, source);
    return source;
}

static c89_source_t *source_for(c89_emit_t *emit, const ssa_module_t *mod, const char *path)
{
    char *it = str_format(emit->arena, "src/%s.c", path);
    fs_file_create(emit->fs, it);

    io_t *io = fs_open(emit->fs, it, eOsAccessWrite | eOsAccessTruncate);
    c89_source_t *source = source_new(io, it, emit->arena);
    map_set(emit->srcmap, mod, source);
    return source;
}

c89_source_t *c89_get_source(c89_emit_t *emit, const ssa_module_t *mod)
{
    if (emit->layout == eFileLayoutPair)
        return emit->source;

    c89_source_t *src = map_get(emit->srcmap, mod);
    CTASSERTF(src != NULL, "no source for %s", mod->name);

    return src;
}

c89_source_t *c89_get_header(c89_emit_t *emit, const ssa_module_t *mod)
{
    if (emit->layout == eFileLayoutPair)
        return emit->header;

    c89_source_t *src = map_get(emit->hdrmap, mod);
    CTASSERTF(src != NULL, "no header for %s", mod->name);

    return src;
}

io_t *c89_get_header_io(c89_emit_t *emit, const ssa_module_t *mod)
{
    c89_source_t *src = c89_get_header(emit, mod);
    return src->io;
}

io_t *c89_get_source_io(c89_emit_t *emit, const ssa_module_t *mod)
{
    c89_source_t *src = c89_get_source(emit, mod);
    return src->io;
}

// begin api

static const char *format_path(const char *base, const char *name, arena_t *arena)
{
    if (ctu_strlen(base) == 0) { return name; }
    return str_format(arena, "%s/%s", base, name);
}

static void collect_deps(c89_emit_t *emit, const ssa_module_t *mod, vector_t *symbols)
{
    size_t len = vector_len(symbols);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_symbol_t *global = vector_get(symbols, i);
        map_set(emit->modmap, global, (ssa_module_t*)mod);
    }
}

static void cplusplus_prelude(io_t *io)
{
    io_printf(io,
        "#ifdef __cplusplus\n"
        "extern \"C\" {\n"
        "#endif\n"
    );
}

static void cplusplus_epilogue(io_t *io)
{
    io_printf(io,
        "#ifdef __cplusplus\n"
        "}\n"
        "#endif\n"
    );
}

static void c89_begin_all(c89_emit_t *emit)
{
    // TODO: use a user provided name
    fs_file_create(emit->fs, "module.c");
    fs_file_create(emit->fs, "module.h");

    io_t *src = fs_open(emit->fs, "module.c", eOsAccessWrite | eOsAccessTruncate);
    io_t *hdr = fs_open(emit->fs, "module.h", eOsAccessWrite | eOsAccessTruncate);

    CTASSERTF(io_error(src) == eOsSuccess, "failed to open module.c (%s)", os_error_string(io_error(src), emit->arena));
    CTASSERTF(io_error(hdr) == eOsSuccess, "failed to open module.h (%s)", os_error_string(io_error(hdr), emit->arena));

    vector_push(&emit->sources, (char*)"module.c");

    io_printf(src, "#include \"module.h\"\n");

    io_printf(hdr, "#pragma once\n");
    io_printf(hdr, "#include <stdbool.h>\n");
    io_printf(hdr, "#include <stdint.h>\n");
    io_printf(hdr, "#include <stddef.h>\n");

    cplusplus_prelude(hdr);

    c89_source_t *src_file = source_new(src, "module.c", emit->arena);
    c89_source_t *hdr_file = source_new(hdr, "module.h", emit->arena);

    emit->source = src_file;
    emit->header = hdr_file;
}

static void c89_end_all(c89_emit_t *emit)
{
    c89_source_t *src = emit->source;
    c89_source_t *hdr = emit->header;
    cplusplus_epilogue(hdr->io);

    io_close(src->io);
    io_close(hdr->io);
}

static void c89_begin_module(c89_emit_t *emit, const ssa_module_t *mod)
{
    // collect all symbols defined in this module
    collect_deps(emit, mod, mod->globals);
    collect_deps(emit, mod, mod->functions);

    // create source and header files
    char *path = begin_module(&emit->emit, emit->fs, mod);

    const char *src_file = format_path(path, mod->name, emit->arena);
    const char *hdr_file = format_path(path, mod->name, emit->arena);

    vector_push(&emit->sources, str_format(emit->arena, "src/%s.c", src_file));

    c89_source_t *src = source_for(emit, mod, src_file);
    c89_source_t *hdr = header_for(emit, mod, hdr_file);

    io_printf(hdr->io, "#pragma once\n");
    io_printf(hdr->io, "#include <stdbool.h>\n");
    io_printf(hdr->io, "#include <stdint.h>\n");
    io_printf(hdr->io, "#include <stddef.h>\n");

    cplusplus_prelude(hdr->io);

    io_printf(src->io, "#include \"%s.h\"\n", hdr_file);
}

// emit api

static const char *format_c89_link(tree_linkage_t linkage)
{
    switch (linkage)
    {
    case eLinkImport: return "extern ";
    case eLinkModule: return "static ";

    case eLinkExport:
    case eLinkEntryCli:
    case eLinkEntryGui:
        return "";

    default: CT_NEVER("unknown linkage %d", linkage);
    }
}

static void get_required_headers(c89_emit_t *emit, set_t *requires, const ssa_module_t *root, vector_t *symbols)
{
    size_t len = vector_len(symbols);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_symbol_t *global = vector_get(symbols, i);
        set_t *deps = map_get(emit->deps, global);
        if (deps == NULL) { continue; }

        set_iter_t iter = set_iter(deps);
        while (set_has_next(&iter))
        {
            const ssa_symbol_t *dep = set_next(&iter);
            const ssa_module_t *dep_mod = map_get(emit->modmap, dep);
            if (dep_mod != root)
            {
                set_add(requires, dep_mod);
            }
        }
    }
}

static void emit_required_headers(c89_emit_t *emit, const ssa_module_t *mod)
{
    // TODO: this is very coarse, we should only add deps to the headers
    // for symbols that are externally visible

    size_t len = vector_len(mod->globals);
    set_t *requires = set_new(CT_MAX(len, 1), kTypeInfoPtr, emit->arena); // set of modules required by this module

    get_required_headers(emit, requires, mod, mod->globals);
    get_required_headers(emit, requires, mod, mod->functions);

    io_t *hdr = c89_get_header_io(emit, mod);
    set_iter_t iter = set_iter(requires);
    while (set_has_next(&iter))
    {
        const ssa_module_t *item = set_next(&iter);
        c89_source_t *dep = c89_get_header(emit, item);
        io_printf(hdr, "#include \"%s\"\n", dep->path);
    }
}

static const char *mangle_symbol_name(c89_emit_t *emit, const ssa_symbol_t *symbol)
{
    switch (symbol->linkage)
    {
    case eLinkEntryCli: return "main";
    case eLinkEntryGui: return "WinMain";
    default: break;
    }

    if (symbol->linkage_string != NULL) { return symbol->linkage_string; }

    if (symbol->name == NULL)
    {
        // this is an anonymous symbol, we need to generate a unique name
        return get_anon_symbol_name(&emit->emit, symbol, "anon");
    }

    return symbol->name;
}

static bool is_entry_point(tree_linkage_t link)
{
    return link == eLinkEntryCli || link == eLinkEntryGui;
}

static const char *format_symbol(c89_emit_t *emit, const ssa_type_t *type, const char *name)
{
    return c89_format_type(emit, type, name, eFormatEmitConst);
}

static char *format_integer_literal(arena_t *arena, const mpz_t value)
{
    if (mpz_fits_sint_p(value))
        return mpz_get_str(NULL, 10, value);

    if (integer_fits_longlong(value))
        return str_format(arena, "%sll", mpz_get_str(NULL, 10, value));

    // integer must be unsigned if we get this far
    if (integer_is_unsigned(value))
        return str_format(arena, "%sull", mpz_get_str(NULL, 10, value));

    CT_NEVER("integer literal %s is too large, this should be caught earlier", mpz_get_str(NULL, 10, value));
}

static char *format_integer_value(arena_t *arena, const ssa_value_t *value)
{
    mpz_t digit;
    ssa_value_get_digit(value, digit);
    return format_integer_literal(arena, digit);
}

static void define_enum(io_t *io, const ssa_type_t *type, c89_emit_t *emit)
{
    // update c89_format_type eTypeEnum when this is changed
    const ssa_type_enum_t it = type->sum;
    size_t len = typevec_len(it.cases);
    const ssa_type_t *underlying = it.underlying;
    char *under = str_format(emit->arena, "%s_underlying_t", type->name);
    const char *tydef = c89_format_type(emit, underlying, under, false);
    io_printf(io, "typedef %s;\n", tydef);

    io_printf(io, "enum %s_cases_t { /* %zu cases */\n", type->name, len);
    if (len == 0)
    {
        io_printf(io, "\te%s_empty = 0,\n", type->name);
    }
    else for (size_t i = 0; i < len; i++)
    {
        const ssa_case_t *field = typevec_offset(it.cases, i);

        // TODO: formalize the name mangling for enum fields
        io_printf(io, "\te%s%s = %s,\n", type->name, field->name, format_integer_literal(emit->arena, field->value));
    }
    io_printf(io, "};\n");
}

static void c89_proto_aggregate(io_t *io, const char *ty, const char *name)
{
    io_printf(io, "%s %s;\n", ty, name);
}

void c89_proto_type(c89_emit_t *emit, io_t *io, const ssa_type_t *type)
{
    switch (type->kind)
    {
    case eTypeStruct:
        c89_proto_aggregate(io, "struct", type->name);
        break;
    case eTypeUnion:
        c89_proto_aggregate(io, "union", type->name);
        break;

    case eTypeEnum:
        define_enum(io, type, emit);
        break;

    default:
        break; // TODO: how should we honor visiblity and aliases?
    }
}

static void write_global(c89_emit_t *emit, io_t *io, const ssa_symbol_t *global)
{
    ssa_storage_t storage = global->storage;
    const char *it = c89_format_storage(emit, global->storage, mangle_symbol_name(emit, global), (storage.quals & eQualConst) ? eFormatIsConst : eFormatEmitNone);
    const char *link = format_c89_link(global->linkage);

    io_printf(io, "%s%s", link, it);
}

void c89_proto_global(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *global)
{
    if (global->visibility == eVisiblePublic)
    {
        CTASSERT(global->linkage != eLinkModule); // TODO: move this check into the checker

        io_t *io = c89_get_header_io(emit, mod);
        write_global(emit, io, global);
        io_printf(io, ";\n");
    }
    else
    {
        io_t *io = c89_get_source_io(emit, mod);
        write_global(emit, io, global);
        io_printf(io, ";\n");
    }
}

void c89_proto_function(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *symbol)
{
    // dont generate prototypes for entry points
    if (is_entry_point(symbol->linkage)) { return; }

    io_t *src = c89_get_source_io(emit, mod);
    io_t *hdr = c89_get_header_io(emit, mod);

    const ssa_type_t *type = symbol->type;
    CTASSERTF(type->kind == eTypeClosure, "expected closure type on %s, got %d", symbol->name, type->kind);

    ssa_type_closure_t closure = type->closure;
    const char *params = c89_format_params(emit, closure.params, closure.variadic);
    const char *result = format_symbol(emit, closure.result, mangle_symbol_name(emit, symbol));

    const char *link = format_c89_link(symbol->linkage);

    io_t *dst = symbol->visibility == eVisiblePublic ? hdr : src;
    io_printf(dst, "%s%s(%s);\n", link, result, params);
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

static void proto_module_types(c89_emit_t *emit, io_t *io, const vector_t *types)
{
    size_t len = vector_len(types);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_type_t *type = vector_get(types, i);
        c89_proto_type(emit, io, type);
    }
}

static void c89_proto_all_types(c89_emit_t *emit, const vector_t *modules)
{
    // forward declare all types
    c89_source_t *header = emit->header;
    for (size_t i = 0; i < vector_len(modules); i++)
    {
        const ssa_module_t *mod = vector_get(modules, i);
        proto_module_types(emit, header->io, mod->types);
    }
}

static void c89_proto_all_globals(c89_emit_t *emit, const vector_t *modules)
{
    size_t len = vector_len(modules);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(modules, i);
        proto_symbols(emit, mod, mod->globals, c89_proto_global);
    }
}

static void c89_proto_all_functions(c89_emit_t *emit, const vector_t *modules)
{
    size_t len = vector_len(modules);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(modules, i);
        proto_symbols(emit, mod, mod->functions, c89_proto_function);
    }
}

static void c89_proto_module(c89_emit_t *emit, const ssa_module_t *mod)
{
    emit_required_headers(emit, mod);

    io_t *hdr = c89_get_header_io(emit, mod);
    proto_module_types(emit, hdr, mod->types);

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
        const ssa_param_t *param = typevec_offset(emit->current->params, operand.param);
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
        const ssa_block_t *bb = operand.vreg_context;
        const ssa_step_t *step = typevec_offset(bb->steps, operand.vreg_index);
        const ssa_type_t *type = map_get(emit->stepmap, step);
        return type;
    }

    default: CT_NEVER("unknown operand kind %d", operand.kind);
    }
}

static void set_step_type(c89_emit_t *emit, const ssa_step_t *step, const ssa_type_t *type)
{
    map_set(emit->stepmap, step, (ssa_type_t*)type);
}

static const char *c89_name_vreg(c89_emit_t *emit, const ssa_step_t *step, const ssa_type_t *type)
{
    set_step_type(emit, step, (ssa_type_t*)type);

    const char *id = str_format(emit->arena, "vreg%s", get_step_name(&emit->emit, step));
    return format_symbol(emit, type, id);
}

static const char *c89_name_vreg_by_operand(c89_emit_t *emit, const ssa_step_t *step, ssa_operand_t operand)
{
    const ssa_type_t *type = get_operand_type(emit, operand);
    return c89_name_vreg(emit, step, type);
}

static const ssa_type_t *get_reg_type(const ssa_type_t *type)
{
    switch (type->kind)
    {
    case eTypePointer: {
        ssa_type_pointer_t pointer = type->pointer;
        return pointer.pointer;
    }

    default: CT_NEVER("expected storage or pointer type, got %s (on %s)", ssa_type_name(type->kind), type->name);
    }
}

static const char *c89_name_load_vreg_by_operand(c89_emit_t *emit, const ssa_step_t *step, ssa_operand_t operand)
{
    const ssa_type_t *type = get_operand_type(emit, operand);
    return c89_name_vreg(emit, step, get_reg_type(type));
}

static const char *operand_type_string(c89_emit_t *emit, ssa_operand_t operand)
{
    const ssa_type_t *type = get_operand_type(emit, operand);
    return type_to_string(type, emit->arena);
}

static const char *c89_format_value(c89_emit_t *emit, const ssa_value_t* value);

static const char *c89_format_pointer(c89_emit_t *emit, const ssa_value_t *value)
{
    if (value->value == eValueRelative)
    {
        ssa_relative_value_t relative = value->relative;
        return str_format(emit->arena, "(%s)", relative.symbol->name);
    }

    ssa_literal_value_t literal = ssa_value_get_literal(value);
    arena_t *arena = emit->arena;
    size_t len = vector_len(literal.data);
    vector_t *result = vector_of(len, arena);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_value_t *element = vector_get(literal.data, i);
        const char *it = c89_format_value(emit, element);
        vector_set(result, i, (char*)it);
    }

    char *joined = str_join(", ", result, arena);
    return str_format(arena, "{ %s }", joined);
}

static const char *c89_format_opaque(c89_emit_t *emit, const ssa_value_t *value)
{
    if (value->value == eValueLiteral)
    {
        ssa_literal_value_t literal = value->literal;
        return str_format(emit->arena, "((void*)%sull)", mpz_get_str(NULL, 10, literal.pointer));
    }

    if (value->value == eValueRelative)
    {
        const ssa_relative_value_t *relative = &value->relative;
        return str_format(emit->arena, "((void*)%s)", relative->symbol->name);
    }

    CT_NEVER("unknown opaque value kind %d", value->value);
}

static const char *c89_format_value(c89_emit_t *emit, const ssa_value_t* value)
{
    const ssa_type_t *type = value->type;
    switch (type->kind)
    {
    case eTypeBool:
        return ssa_value_get_bool(value) ? "true" : "false";
    case eTypeDigit:
        return format_integer_value(emit->arena, value);
    case eTypePointer: return c89_format_pointer(emit, value);
    case eTypeOpaque: return c89_format_opaque(emit, value);
    default: CT_NEVER("unknown type kind %d", type->kind);
    }
}

static const char *get_local_name(c89_emit_t *emit, const ssa_local_t *local)
{
    if (local->name != NULL)
        return str_format(emit->arena, "local_%s", local->name);

    return get_anon_local_name(&emit->emit, local, "local_");
}

static const char *c89_format_local(c89_emit_t *emit, size_t local)
{
    typevec_t *locals = emit->current->locals;
    CTASSERTF(local < typevec_len(locals), "local(%zu) > locals(%zu)", local, typevec_len(locals));

    const ssa_local_t *it = typevec_offset(locals, local);
    return get_local_name(emit, it);
}

static const char *c89_format_param(c89_emit_t *emit, size_t param)
{
    typevec_t *params = emit->current->params;
    CTASSERTF(param < typevec_len(params), "param(%zu) > params(%zu)", param, typevec_len(params));

    const ssa_param_t *it = typevec_offset(params, param);
    return it->name;
}

static const char *c89_format_operand(c89_emit_t *emit, ssa_operand_t operand)
{
    switch (operand.kind)
    {
    case eOperandEmpty: return "/* empty */";
    case eOperandImm:
        return c89_format_value(emit, operand.value);

    case eOperandBlock:
        return str_format(emit->arena, "bb%s", get_block_name(&emit->emit, operand.bb));

    case eOperandReg:
        return str_format(emit->arena, "vreg%s", get_step_from_block(&emit->emit, operand.vreg_context, operand.vreg_index));

    case eOperandGlobal:
        return mangle_symbol_name(emit, operand.global);

    case eOperandFunction:
        return mangle_symbol_name(emit, operand.function);

    case eOperandLocal:
        return c89_format_local(emit, operand.local);

    case eOperandParam:
        return c89_format_param(emit, operand.param);

    default: CT_NEVER("unknown operand kind %d", operand.kind);
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

static void c89_write_address(c89_emit_t *emit, io_t *io, const ssa_step_t *step)
{
    ssa_addr_t addr = step->addr;
    ssa_operand_t symbol = addr.symbol;
    const ssa_type_t *type = get_operand_type(emit, symbol);

    const ssa_type_t *ptr = ssa_type_pointer(type->name, eQualNone, (ssa_type_t*)type, 0);
    const char *step_name = c89_name_vreg(emit, step, ptr);

    io_printf(io, "\t%s = &(%s); /* %s */\n",
        step_name,
        c89_format_operand(emit, addr.symbol),
        type_to_string(ptr, emit->arena)
    );
}

static void c89_write_offset(c89_emit_t *emit, io_t *io, const ssa_step_t *step)
{
    ssa_offset_t offset = step->offset;
    io_printf(io, "\t%s = &%s[%s]; /* (array = %s, offset = %s) */\n",
        c89_name_vreg_by_operand(emit, step, offset.array),
        c89_format_operand(emit, offset.array),
        c89_format_operand(emit, offset.offset),
        operand_type_string(emit, offset.array),
        operand_type_string(emit, offset.offset)
    );
}

static void c89_write_member(c89_emit_t *emit, io_t *io, const ssa_step_t *step)
{
    CTASSERTF(step->opcode == eOpMember, "expected member, got %s", ssa_opcode_name(step->opcode));

    ssa_member_t member = step->member;
    const ssa_type_t *record_ptr = get_operand_type(emit, member.object);
    CTASSERTF(record_ptr->kind == eTypePointer, "expected record type, got %s", type_to_string(record_ptr, emit->arena));

    ssa_type_pointer_t ptr = record_ptr->pointer;
    const ssa_type_t *record = ptr.pointer;
    CTASSERTF(record->kind == eTypeStruct, "expected record type, got %s", type_to_string(record, emit->arena));

    ssa_type_record_t record_type = record->record;
    const ssa_field_t *field = typevec_offset(record_type.fields, member.index);

    io_printf(io, "\t%s = &%s->%s;\n",
        c89_name_vreg(emit, step, ssa_type_pointer(field->name, eQualNone, (ssa_type_t*)field->type, 1)),
        c89_format_operand(emit, member.object),
        field->name
    );
}

static void c89_write_block(c89_emit_t *emit, io_t *io, const ssa_block_t *bb)
{
    size_t len = typevec_len(bb->steps);
    io_printf(io, "bb%s: { /* len = %zu */\n", get_block_name(&emit->emit, bb), len);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_step_t *step = typevec_offset(bb->steps, i);
        switch (step->opcode)
        {
        case eOpNop:
            io_printf(io, "\t/* nop */\n");
            break;
        case eOpValue: {
            const ssa_value_t *value = step->value;
            const char *name = c89_name_vreg(emit, step, value->type);
            io_printf(io, "\t%s = %s;\n", name, c89_format_value(emit, value));
            break;
        }
        case eOpStore: {
            ssa_store_t store = step->store;
            io_printf(io, "\t*(%s) = %s;\n",
                c89_format_operand(emit, store.dst),
                c89_format_operand(emit, store.src)
            );
            break;
        }
        case eOpCast: {
            ssa_cast_t cast = step->cast;
            io_printf(io, "\t%s = (%s)(%s);\n",
                c89_name_vreg(emit, step, cast.type),
                format_symbol(emit, cast.type, NULL),
                c89_format_operand(emit, cast.operand)
            );
            break;
        }
        case eOpLoad: {
            ssa_load_t load = step->load;
            io_printf(io, "\t%s = *(%s);\n",
                c89_name_load_vreg_by_operand(emit, step, load.src),
                c89_format_operand(emit, load.src)
            );
            break;
        }

        case eOpAddress:
            c89_write_address(emit, io, step);
            break;
        case eOpOffset:
            c89_write_offset(emit, io, step);
            break;
        case eOpMember:
            c89_write_member(emit, io, step);
            break;

        case eOpUnary: {
            ssa_unary_t unary = step->unary;
            io_printf(io, "\t%s = (%s %s);\n",
                c89_name_vreg_by_operand(emit, step, unary.operand),
                unary_symbol(unary.unary),
                c89_format_operand(emit, unary.operand)
            );
            break;
        }
        case eOpBinary: {
            ssa_binary_t bin = step->binary;
            io_printf(io, "\t%s = (%s %s %s);\n",
                c89_name_vreg_by_operand(emit, step, bin.lhs),
                c89_format_operand(emit, bin.lhs),
                binary_symbol(bin.binary),
                c89_format_operand(emit, bin.rhs)
            );
            break;
        }
        case eOpCompare: {
            ssa_compare_t cmp = step->compare;
            io_printf(io, "\t%s = (%s %s %s);\n",
                c89_name_vreg(emit, step, ssa_type_bool("bool", eQualConst)),
                c89_format_operand(emit, cmp.lhs),
                compare_symbol(cmp.compare),
                c89_format_operand(emit, cmp.rhs)
            );
            break;
        }

        case eOpCall: {
            ssa_call_t call = step->call;
            size_t args_len = typevec_len(call.args);

            const ssa_type_t *ty = get_operand_type(emit, call.function);
            ssa_type_closure_t closure = ty->closure;
            const ssa_type_t *result = closure.result;

            vector_t *args = vector_of(args_len, emit->arena);
            for (size_t arg_idx = 0; arg_idx < args_len; arg_idx++)
            {
                const ssa_operand_t *operand = typevec_offset(call.args, arg_idx);
                vector_set(args, arg_idx, (char*)c89_format_operand(emit, *operand));
            }

            io_printf(io, "\t");

            if (result->kind != eTypeEmpty && result->kind != eTypeUnit)
            {
                io_printf(io, "%s = ", c89_name_vreg(emit, step, result));
            }

            io_printf(io, "%s(%s);\n",
                c89_format_operand(emit, call.function),
                str_join(", ", args, emit->arena)
            );
            break;
        }

        case eOpJump: {
            ssa_jump_t jmp = step->jump;
            io_printf(io, "\tgoto %s;\n", c89_format_operand(emit, jmp.target));
            break;
        }
        case eOpBranch: {
            ssa_branch_t br = step->branch;
            io_printf(io, "\tif (%s) { goto %s; }", c89_format_operand(emit, br.cond), c89_format_operand(emit, br.then));
            if (!operand_is_empty(br.other))
            {
                io_printf(io, " else { goto %s; }", c89_format_operand(emit, br.other));
            }
            io_printf(io, "\n");
            break;
        }
        case eOpReturn: {
            ssa_return_t ret = step->ret;
            if (!operand_cant_return(ret.value))
            {
                io_printf(io, "\treturn %s;\n", c89_format_operand(emit, ret.value));
            }
            else
            {
                io_printf(io, "\treturn;\n");
            }
            break;
        }

        default: CT_NEVER("unknown opcode %d", step->opcode);
        }
    }
    io_printf(io, "} /* end %s */\n", get_block_name(&emit->emit, bb));
}

/// defines

static void define_record(c89_emit_t *emit, const char *aggregate, io_t *io, const ssa_type_t *type)
{
    const ssa_type_record_t record = type->record;
    io_printf(io, "%s %s {\n", aggregate, type->name);
    size_t len = typevec_len(record.fields);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_field_t *field = typevec_offset(record.fields, i);
        io_printf(io, "\t%s;\n", format_symbol(emit, field->type, field->name));
    }
    io_printf(io, "};\n");
}

void c89_define_type(c89_emit_t *emit, io_t *io, const ssa_type_t *type)
{
    switch (type->kind)
    {
    case eTypeStruct:
        define_record(emit, "struct", io, type);
        break;
    case eTypeUnion:
        define_record(emit, "union", io, type);
        break;

    default:
        break;
    }
}

static void write_init(c89_emit_t *emit, io_t *io, const ssa_value_t *value)
{
    const ssa_type_t *type = value->type;
    const char *init = c89_format_value(emit, value);

    if (type->kind == eTypePointer)
    {
        io_printf(io, " = %s", init);
    }
    else
    {
       io_printf(io, " = { %s }", init);
    }
}

void c89_define_global(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *symbol)
{
    io_t *src = c89_get_source_io(emit, mod);

    if (symbol->linkage != eLinkImport)
    {
        const ssa_value_t *value = symbol->value;
        write_global(emit, src, symbol);
        if (value->init)
        {
            write_init(emit, src, value);
        }

        io_printf(src, ";\n");
    }
}

static void write_locals(c89_emit_t *emit, io_t *io, typevec_t *locals)
{
    size_t len = typevec_len(locals);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_local_t *local = typevec_offset(locals, i);
        const char *name = get_local_name(emit, local);
        io_printf(io, "\t%s;\n",
            c89_format_storage(emit, local->storage, name, eFormatEmitNone)
        );
    }
}

void c89_define_function(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *symbol)
{
    io_t *src = c89_get_source_io(emit, mod);

    const ssa_type_t *type = symbol->type;
    CTASSERTF(type->kind == eTypeClosure, "expected closure type on %s, got %d", symbol->name, type->kind);

    ssa_type_closure_t closure = type->closure;
    const char *params = c89_format_params(emit, closure.params, closure.variadic);
    const char *result = format_symbol(emit, closure.result, mangle_symbol_name(emit, symbol));

    const char *link = format_c89_link(symbol->linkage);

    if (symbol->linkage != eLinkImport)
    {
        io_printf(src, "%s%s(%s) {\n", link, result, params);
        write_locals(emit, src, symbol->locals);
        io_printf(src, "\tgoto bb%s;\n", get_block_name(&emit->emit, symbol->entry));
        size_t len = vector_len(symbol->blocks);
        for (size_t i = 0; i < len; i++)
        {
            const ssa_block_t *bb = vector_get(symbol->blocks, i);
            c89_write_block(emit, src, bb);
        }
        io_printf(src, "}\n");

        map_reset(emit->stepmap);
        counter_reset(&emit->emit);
    }
}

static void define_symbols(
    c89_emit_t *emit,
    const ssa_module_t *mod,
    vector_t *vec,
    void (*fn)(c89_emit_t*, const ssa_module_t*, const ssa_symbol_t*)
)
{
    size_t len = vector_len(vec);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_symbol_t *symbol = vector_get(vec, i);
        emit->current = symbol;
        fn(emit, mod, symbol);
    }
}

static void define_type_ordererd(c89_emit_t *emit, io_t *io, const ssa_type_t *type)
{
    if (set_contains(emit->defined, type)) { return; }
    set_add(emit->defined, type);

    // TODO: this is probably a touch broken, types may be put into the wrong translation unit
    if (type->kind == eTypeStruct || type->kind == eTypeUnion)
    {
        // only match struct and union as they can have dependencies
        ssa_type_record_t record = type->record;
        size_t len = typevec_len(record.fields);
        for (size_t i = 0; i < len; i++)
        {
            const ssa_field_t *field = typevec_offset(record.fields, i);
            define_type_ordererd(emit, io, field->type);
        }
    }

    c89_define_type(emit, io, type);
}

static void c89_define_types(c89_emit_t *emit, io_t *io, vector_t *types)
{
    size_t len = vector_len(types);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_type_t *type = vector_get(types, i);
        define_type_ordererd(emit, io, type);
    }
}

static void c89_define_module(c89_emit_t *emit, const ssa_module_t *mod)
{
    io_t *hdr = c89_get_header_io(emit, mod);
    c89_define_types(emit, hdr, mod->types);
    define_symbols(emit, mod, mod->globals, c89_define_global);
    define_symbols(emit, mod, mod->functions, c89_define_function);

    cplusplus_epilogue(hdr);
}

static void c89_define_all_types(c89_emit_t *emit, const vector_t *modules)
{
    size_t len = vector_len(modules);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(modules, i);
        c89_define_types(emit, emit->header->io, mod->types);
    }
}

static void c89_define_all_globals(c89_emit_t *emit, const vector_t *modules)
{
    size_t len = vector_len(modules);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(modules, i);
        define_symbols(emit, mod, mod->globals, c89_define_global);
    }
}

static void c89_define_all_functions(c89_emit_t *emit, const vector_t *modules)
{
    size_t len = vector_len(modules);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(modules, i);
        define_symbols(emit, mod, mod->functions, c89_define_function);
    }
}

emit_result_t cfamily_ssa(target_runtime_t *runtime, const ssa_result_t *ssa, target_emit_t *emit)
{
    size_t len = vector_len(ssa->modules);

    arena_t *arena = runtime->arena;
    vector_t *modules = ssa->modules;

    c89_emit_t ctx = {
        .arena = arena,
        .emit = {
            .arena = arena,
            .reports = runtime->logger,
            .block_names = names_new(64, arena),
            .vreg_names = names_new(64, arena),
            .anon_names = names_new(64, arena),
        },
        .modmap = map_optimal(len * 2, kTypeInfoPtr, arena),
        .srcmap = map_optimal(len, kTypeInfoPtr, arena),
        .hdrmap = map_optimal(len, kTypeInfoPtr, arena),

        .stepmap = map_optimal(64, kTypeInfoPtr, arena),
        .defined = set_new(64, kTypeInfoPtr, arena),

        .fs = emit->fs,
        .deps = ssa->deps,
        .sources = vector_new(32, arena),
        .layout = emit->layout,
    };

    if (emit->layout == eFileLayoutPair)
    {
        c89_begin_all(&ctx);

        c89_proto_all_types(&ctx, modules);
        c89_proto_all_globals(&ctx, modules);
        c89_proto_all_functions(&ctx, modules);

        c89_define_all_types(&ctx, modules);
        c89_define_all_globals(&ctx, modules);
        c89_define_all_functions(&ctx, modules);

        c89_end_all(&ctx);
    }
    else
    {
        for (size_t i = 0; i < len; i++)
        {
            const ssa_module_t *mod = vector_get(modules, i);
            c89_begin_module(&ctx, mod);
        }

        for (size_t i = 0; i < len; i++)
        {
            const ssa_module_t *mod = vector_get(modules, i);
            c89_proto_module(&ctx, mod);
        }

        for (size_t i = 0; i < len; i++)
        {
            const ssa_module_t *mod = vector_get(modules, i);
            c89_define_module(&ctx, mod);
        }
    }

    emit_result_t result = {
        .files = ctx.sources,
    };

    return result;
}
