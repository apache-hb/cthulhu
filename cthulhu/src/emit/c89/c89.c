#include "c89.h"

#include "report/report.h"

#include "io/fs.h"

#include "std/str.h"
#include "std/map.h"
#include "std/vector.h"
#include "std/typevec.h"

#include "base/panic.h"
#include "base/memory.h"

#include <string.h>

typedef struct c89_t {
    emit_t emit;

    fs_t *fs;
    c89_flags_t flags;

    map_t *modules; // map<ssa_module_t*, c89_module_t*>

    vector_t *path;
} c89_t;

typedef struct c89_file_t {
    const char *path;
    io_t *io;
} c89_file_t;

typedef struct c89_module_t {
    c89_file_t source;
    c89_file_t include;
} c89_module_t;

typedef struct c89_context_t {
    c89_t *emit;

    io_t *inc;
    io_t *src;
} c89_context_t;

static io_t *get_source(c89_module_t *mod)
{
    return mod->source.io;
}

static io_t *get_include(c89_module_t *mod)
{
    return mod->include.io;
}

static c89_file_t create_new_file(const char *path, io_t *io)
{
    c89_file_t result = {
        .path = path,
        .io = io,
    };

    return result;
}

static c89_module_t *create_new_module(c89_file_t src, c89_file_t inc)
{
    c89_module_t *result = ctu_malloc(sizeof(c89_module_t));
    result->source = src;
    result->include = inc;
    return result;
}

static const char *join_path(c89_t *emit, vector_t *path, const char *name)
{
    const char *join = (emit->flags & eEmitFlat) ? "." : "/";

    return (vector_len(path) > 0)
        ? format("%s%s%s", str_join(join, path), join, name)
        : name;
}

static c89_module_t *create_module_file(c89_t *emit, const char *root, vector_t *path, ssa_module_t *mod)
{
    char *sourceFile = format("src/%s.c", root);
    char *includeFile = format("include/%s.h", root);

    fs_file_create(emit->fs, sourceFile);
    fs_file_create(emit->fs, includeFile);

    io_t *src = fs_open(emit->fs, sourceFile, eAccessWrite | eAccessText);
    io_t *inc = fs_open(emit->fs, includeFile, eAccessWrite | eAccessText);

    write_string(inc, "#pragma once\n");
    write_string(inc, "#include <stdbool.h>\n");
    write_string(inc, "#include <stdint.h>\n");

    write_string(src, "#include \"%s.h\"\n", root);

    const char *id = (vector_len(path) > 0)
        ? format("%s.%s", str_join(".", path), mod->name)
        : mod->name;

    write_string(inc, "\n/**\n * module %s\n */\n", id);

    c89_file_t incFd = create_new_file(includeFile, inc);
    c89_file_t srcFd = create_new_file(sourceFile, src);

    c89_module_t *self = create_new_module(srcFd, incFd);

    map_set_ptr(emit->modules, mod, self);

    return self;
}

static const char *type_to_string(c89_t *emit, const ssa_type_t *type, const char *name);

static const char *get_digit_name(ssa_type_digit_t info)
{
    switch (info.digit)
    {
    case eDigitChar: return (info.sign == eSignUnsigned) ? "unsigned char" : "signed char";
    case eDigitShort: return (info.sign == eSignUnsigned) ? "unsigned short" : "signed short";
    case eDigitInt: return (info.sign == eSignUnsigned) ? "unsigned int" : "signed int";
    case eDigitLong: return (info.sign == eSignUnsigned) ? "unsigned long long" : "signed long long";
    case eDigitPtr: return (info.sign == eSignUnsigned) ? "uintptr_t" : "intptr_t";
    case eDigitSize: return (info.sign == eSignUnsigned) ? "size_t" : "ptrdiff_t";
    case eDigitMax: return (info.sign == eSignUnsigned) ? "uintmax_t" : "intmax_t";
    default: NEVER("invalid digit");
    }
}

static const char *qual_to_string(c89_t *emit, ssa_type_qualify_t qual, const char *name)
{
    quals_t quals = qual.quals;
    vector_t *parts = vector_new(3);

    if (!(quals & eQualMutable))
    {
        vector_push(&parts, (char*)"const");
    }

    if (quals & eQualVolatile)
    {
        vector_push(&parts, (char*)"volatile");
    }

    if (quals & eQualAtomic)
    {
        vector_push(&parts, (char*)"_Atomic");
    }

    const char *inner = type_to_string(emit, qual.type, name);
    if (vector_len(parts) == 0)
    {
        return inner;
    }

    const char *decorate = str_join(" ", parts);
    return format("%s %s", decorate, inner);
}

static const char *closure_to_string(c89_t *emit, ssa_type_closure_t closure, const char *name)
{
    size_t len = typevec_len(closure.params);
    vector_t *args = vector_of(len);

    for (size_t i = 0; i < len; i++)
    {
        ssa_param_t *param = typevec_offset(closure.params, i);
        const char *type = type_to_string(emit, param->type, param->name);

        vector_push(&args, (char*)type);
    }

    const char *params = str_join(", ", args);
    const char *ret = type_to_string(emit, closure.result, NULL);

    return format("%s (*%s)(%s)", ret, (name == NULL) ? "" : name, params);
}

static const char *type_to_string(c89_t *emit, const ssa_type_t *type, const char *name)
{
    CTASSERT(type != NULL);

    switch (type->kind)
    {
    case eTypeEmpty: NEVER("empty type indicates unreachable");
    case eTypeUnit: return (name == NULL) ? "void" : format("void %s", name);
    case eTypeBool: return (name == NULL) ? "bool" : format("bool %s", name);
    case eTypeDigit: return (name == NULL) ? get_digit_name(type->digit) : format("%s %s", get_digit_name(type->digit), name);
    case eTypeString: return (name == NULL) ? "const char*" : format("const char *%s", name);
    case eTypeQualify: return qual_to_string(emit, type->qualify, name);
    case eTypeClosure: return closure_to_string(emit, type->closure, name);
    default: NEVER("invalid type kind");
    }
}

static const char *value_to_string(c89_t *emit, const ssa_value_t *value)
{
    CTASSERT(value != NULL);
    const ssa_type_t *type = value->type;
    switch (type->kind)
    {
    case eTypeBool: return (value->boolValue) ? "true" : "false";
    case eTypeDigit: return mpz_get_str(NULL, 10, value->digitValue);
    case eTypeString: return format("\"%s\"", str_normalizen(value->stringValue, value->stringLength));
    default: NEVER("invalid value kind %d", type->kind);
    }
}

static const char *format_step_name(c89_t *emit, const ssa_step_t *step)
{
    const ssa_type_t *type = step->type;
    char *name = format("reg%s", get_step_name(&emit->emit, step));
    return type_to_string(emit, type, name);
}

static const char *operand_to_string(c89_t *emit, ssa_operand_t operand)
{
    switch (operand.kind)
    {
    case eOperandImm:
        return value_to_string(emit, operand.value);
    case eOperandBlock:
        return format("block%s", get_block_name(&emit->emit, operand.bb));
    case eOperandReg:
        return format("reg%s", get_step_from_block(&emit->emit, operand.vregContext, operand.vregIndex));
    case eOperandGlobal:
        return operand.global->name;
    case eOperandFunction:
        return operand.function->name;
    case eOperandLocal:
        return format("local%zu", operand.local);

    default: NEVER("invalid operand kind %d", operand.kind);
    }
}

// can this type be materialized in a register?
static bool material_type(const ssa_type_t *type)
{
    return (type->kind == eTypeEmpty) || (type->kind == eTypeUnit);
}

static bool operand_is_return(ssa_operand_t operand)
{
    if (operand.kind != eOperandImm) { return false; }

    const ssa_value_t *value = operand.value;
    return material_type(value->type);
}

static void write_step(c89_t *emit, io_t *io, const ssa_step_t *step)
{
    switch (step->opcode)
    {
    case eOpLoad: {
        ssa_load_t load = step->load;
        write_string(io, "\t%s = %s;\n", format_step_name(emit, step), operand_to_string(emit, load.src));
        break;
    }
    case eOpStore: {
        ssa_store_t store = step->store;
        write_string(io, "\t%s = %s;\n", operand_to_string(emit, store.dst), operand_to_string(emit, store.src));
        break;
    }
    case eOpJump: {
        ssa_jump_t jump = step->jump;
        write_string(io, "\tgoto %s;\n", operand_to_string(emit, jump.target));
        break;
    }
    case eOpReturn: {
        ssa_return_t ret = step->ret;
        if (operand_is_return(ret.value))
        {
            write_string(io, "\treturn;\n");
        }
        else
        {
            write_string(io, "\treturn %s;\n", operand_to_string(emit, ret.value));
        }
        break;
    }
    case eOpBinary: {
        ssa_binary_t binary = step->binary;
        write_string(io, "\t%s = %s %s %s;\n",
            format_step_name(emit, step),
            operand_to_string(emit, binary.lhs),
            binary_symbol(binary.binary),
            operand_to_string(emit, binary.rhs)
        );
        break;
    }
    case eOpCall: {
        ssa_call_t call = step->call;
        const char *fn = operand_to_string(emit, call.function);
        size_t len = typevec_len(call.args);
        vector_t *args = vector_of(len);
        for (size_t i = 0; i < len; i++)
        {
            ssa_operand_t operand;
            typevec_get(call.args, i, &operand);
            vector_set(args, i, (char*)operand_to_string(emit, operand));
        }
        char *str = str_join(", ", args);
        if (material_type(step->type))
        {
            write_string(io, "\t%s = %s(%s);\n", format_step_name(emit, step), fn, str);
        }
        else
        {
            write_string(io, "\t%s(%s);\n", fn, str);
        }
        break;
    }
    default: NEVER("invalid opcode %d", step->opcode);
    }
}

static void write_block(c89_t *emit, io_t *io, const ssa_block_t *block)
{
    size_t len = typevec_len(block->steps);
    char *name = get_block_name(&emit->emit, block);
    write_string(io, "block%s: // [len=%zu]\n", name, len);

    for (size_t i = 0; i < len; i++)
    {
        ssa_step_t *step = typevec_offset(block->steps, i);
        write_step(emit, io, step);
    }
}

static void write_function(c89_context_t *ctx, const ssa_symbol_t *fn)
{
    c89_t *emit = ctx->emit;
    const ssa_type_t *signature = fn->type;
    const ssa_type_closure_t closure = signature->closure;
    const char *name = fn->mangle != NULL ? fn->mangle : fn->name;

    const char *result = type_to_string(emit, closure.result, name);
    size_t len = typevec_len(closure.params);
    vector_t *params = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        ssa_param_t *param = typevec_offset(closure.params, i);
        const char *type = type_to_string(emit, param->type, param->name);
        vector_set(params, i, (char*)type);
    }

    char *args = str_join(", ", params);

    if (closure.variadic) { args = format("%s, ...", args); }

    bool isImported = fn->entry == NULL;

    if (isImported)
    {
        switch (fn->visible)
        {
        case eVisiblePublic:
            write_string(ctx->inc, "extern %s(%s);\n", result, args);
            break;
        case eVisiblePrivate:
            write_string(ctx->src, "extern %s(%s);\n", result, args);
            break;
        default: NEVER("invalid visibility");
        }

        return;
    }

    switch (fn->visible)
    {
    case eVisiblePublic:
        write_string(ctx->inc, "%s(%s);\n", result, args);
        write_string(ctx->src, "%s(%s)\n{\n", result, args);
        break;
    case eVisiblePrivate:
        write_string(ctx->src, "static %s(%s)\n{\n", result, args);
        break;
    default: NEVER("invalid visibility");
    }

    size_t locals = typevec_len(fn->locals);
    for (size_t i = 0; i < locals; i++)
    {
        ssa_local_t local;
        typevec_get(fn->locals, i, &local);
        const char *type = type_to_string(emit, local.type, format("local%zu", i));
        write_string(ctx->src, "\t%s;\n", type);
    }

    size_t blocks = vector_len(fn->blocks);
    for (size_t i = 0; i < blocks; i++)
    {
        ssa_block_t *block = vector_get(fn->blocks, i);
        write_block(ctx->emit, ctx->src, block);
    }

    write_string(ctx->src, "}\n\n");
}

static void create_module_dir(c89_t *emit, ssa_module_t *mod)
{
    const char *name = mod->name;
    CTASSERT(name != NULL);

    const char *path = join_path(emit, emit->path, name);

    c89_module_t *it = create_module_file(emit, path, emit->path, mod);

    io_t *inc = get_include(it);
    io_t *src = get_source(it);

    map_iter_t globals = map_iter(mod->globals);
    while (map_has_next(&globals))
    {
        map_entry_t entry = map_next(&globals);
        ssa_symbol_t *global = entry.value;

        CTASSERT(global->value != NULL);

        const char *type = type_to_string(emit, global->type, global->name);
        const char *value = value_to_string(emit, global->value);

        switch (global->visible)
        {
        case eVisiblePublic:
            write_string(inc, "extern %s;\n", type);
            write_string(src, "%s = %s;\n", type, value);
            break;
        case eVisiblePrivate:
            write_string(src, "static %s = %s;\n", type, value);
            break;

        default: NEVER("invalid visibility");
        }
    }

    map_iter_t functions = map_iter(mod->functions);
    while (map_has_next(&functions))
    {
        map_entry_t entry = map_next(&functions);
        ssa_symbol_t *function = entry.value;
        c89_context_t ctx = {
            .emit = emit,
            .inc = inc,
            .src = src,
        };
        write_function(&ctx, function);
    }

    if (map_empty(mod->modules))
    {
        return;
    }

    char *includeDir = format("include/%s", path);
    fs_dir_create(emit->fs, includeDir);

    char *sourceDir = format("src/%s", path);
    fs_dir_create(emit->fs, sourceDir);

    vector_push(&emit->path, (char*)name);

    map_iter_t iter = map_iter(mod->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        create_module_dir(emit, entry.value);
    }

    vector_drop(emit->path);
}

static void create_root_dir(c89_t *emit, ssa_module_t *mod)
{
    map_iter_t iter = map_iter(mod->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        ssa_module_t *child = entry.value;
        create_module_dir(emit, child);
    }
}

static void close_all(map_t *modMap)
{
    map_iter_t iter = map_iter(modMap);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        c89_module_t *mod = entry.value;
        io_close(mod->source.io);
        io_close(mod->include.io);
    }
}

c89_emit_result_t emit_c89(const c89_emit_options_t *options)
{
    emit_options_t opts = options->opts;
    c89_t c89 = {
        .emit = {
            .reports = opts.reports,
            .blockNames = names_new(64),
            .vregNames = names_new(64),
        },
        .fs = opts.fs,
        .flags = options->flags,
        .modules = map_optimal(64),

        .path = vector_new(4)
    };

    if (c89.flags & eEmitFlat)
    {
        CTASSERTF(c89.flags & eEmitHeaders, "eEmitFlat requires eEmitHeaders");
    }

    fs_dir_create(c89.fs, "include");
    fs_dir_create(c89.fs, "src");

    create_root_dir(&c89, opts.mod);

    vector_t *sources = map_values(c89.modules);
    size_t len = vector_len(sources);
    vector_t *paths = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        c89_module_t *mod = vector_get(sources, i);
        c89_file_t src = mod->source;
        vector_set(paths, i, (char*)src.path);
    }

    close_all(c89.modules);

    c89_emit_result_t result = {
        .sources = paths
    };

    return result;
}
