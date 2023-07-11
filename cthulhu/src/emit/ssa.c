#include "common.h"

#include "std/map.h"
#include "std/str.h"
#include "std/vector.h"
#include "std/set.h"

#include "std/typed/vector.h"

#include "base/panic.h"

#include "report/report.h"

#include "io/fs.h"

#include <string.h>

typedef struct ssa_t {
    emit_t emit;
    fs_t *fs;

    map_t *modules;
} ssa_t;
#if 0
static const char *ssa_type_to_string(const ssa_type_t *type);

static const char *ssa_digit_to_string(ssa_type_digit_t digit)
{
    return format("digit(digit=%s, sign=%s)", digit_name(digit.digit), sign_name(digit.sign));
}

static const char *ssa_closure_to_string(ssa_type_closure_t closure)
{
    const char *result = ssa_type_to_string(closure.result);
    size_t len = typevec_len(closure.params);
    vector_t *params = vector_of(len);

    ssa_param_t param = { NULL, NULL };
    for (size_t i = 0; i < len; i++)
    {
        typevec_get(closure.params, i, &param);
        const char *type = ssa_type_to_string(param.type);
        vector_set(params, i, format("%s: %s", param.name, type));
    }

    char *args = str_join(", ", params);
    return format("closure(result: %s, args: [%s])", result, args);
}

static const char *ssa_qualify_to_string(ssa_type_qualify_t qualify)
{
    const char *type = ssa_type_to_string(qualify.type);
    const char *quals = quals_name(qualify.quals);
    return format("qualify(type: %s, quals: %s)", type, quals);
}

static const char *ssa_type_to_string(const ssa_type_t *type)
{
    switch (type->kind)
    {
    case eTypeEmpty: return "empty";
    case eTypeUnit: return "unit";
    case eTypeBool: return "bool";
    case eTypeDigit: return ssa_digit_to_string(type->digit);
    case eTypeString: return "string";
    case eTypeClosure: return ssa_closure_to_string(type->closure);
    case eTypeQualify: return ssa_qualify_to_string(type->qualify);
    default: NEVER("Invalid type kind: %d", type->kind);
    }
}

static const char *ssa_value_to_string(const ssa_value_t *value)
{
    const ssa_type_t *type = value->type;

    switch (type->kind)
    {
    case eTypeEmpty:
    case eTypeUnit:
        return ssa_type_to_string(type);

    case eTypeBool:
        return format("bool(value: %s)", value->boolValue ? "true" : "false");
    case eTypeDigit:
        return format("digit(value: %s, type: %s)", mpz_get_str(NULL, 10, value->digitValue), ssa_type_to_string(type));

    case eTypeString:
        return format("string(value: \"%s\")", str_normalizen(value->stringValue, value->stringLength));

    default: NEVER("Invalid value kind: %d", type->kind);
    }
}

static const char *ssa_imm_string(const ssa_value_t *value)
{
    const ssa_type_t *type = value->type;
    switch (type->kind)
    {
    case eTypeEmpty: return format("empty(type: %s)", ssa_type_to_string(type));
    case eTypeUnit: return format("unit(type: %s)", ssa_type_to_string(type));
    case eTypeString: {
        const char *str = str_normalizen(value->stringValue, value->stringLength);
        return format("string(value: \"%s\", type: %s)", str, ssa_type_to_string(type));
    }
    case eTypeBool: return value->boolValue ? "true" : "false";
    case eTypeDigit: return mpz_get_str(NULL, 10, value->digitValue);

    default: NEVER("Invalid imm kind: %d", type->kind);
    }
}

static const char *ssa_operand_string(ssa_t *ssa, ssa_operand_t operand)
{
    switch (operand.kind)
    {
    case eOperandEmpty: return "";
    case eOperandLocal: return format("local.%zu", operand.local);
    case eOperandParam: return format("param.%zu", operand.param);
    case eOperandGlobal: {
        const ssa_symbol_t *sym = operand.global;
        return format("&%s", sym->name);
    }
    case eOperandFunction: {
        const ssa_symbol_t *sym = operand.function;
        return format("&fn(%s)", sym->name);
    }
    case eOperandReg: {
        const char *reg = get_step_from_block(&ssa->emit, operand.vregContext, operand.vregIndex);
        return format("%%%s", reg);
    }
    case eOperandImm: {
        const char *imm = ssa_imm_string(operand.value);
        return format("$%s", imm);
    }
    case eOperandBlock: {
        return format(".%s", get_block_name(&ssa->emit, operand.bb));
    }

    default: NEVER("Invalid operand kind: %d", operand.kind);
    }
}

static void write_step(io_t *io, ssa_t *ssa, const ssa_step_t *step)
{
    switch (step->opcode)
    {
    case eOpReturn: {
        ssa_return_t ret = step->ret;
        const char *value = ssa_operand_string(ssa, ret.value);
        write_string(io, "\tret %s\n", value);
        break;
    }
    case eOpImm: {
        ssa_imm_t imm = step->imm;
        const char *value = ssa_imm_string(imm.value);
        write_string(io, "\t%%%s = imm %s\n", get_step_name(&ssa->emit, step), value);
        break;
    }
    case eOpJump: {
        ssa_jump_t jump = step->jump;
        const char *target = ssa_operand_string(ssa, jump.target);
        write_string(io, "\tjmp %s\n", target);
        break;
    }
    case eOpAddress: {
        ssa_addr_t addr = step->addr;
        const char *target = ssa_operand_string(ssa, addr.symbol);
        write_string(io, "\t%%%s = addr %s\n", get_step_name(&ssa->emit, step), target);
        break;
    }
    case eOpLoad: {
        ssa_load_t load = step->load;
        const char *target = ssa_operand_string(ssa, load.src);
        write_string(io, "\t%%%s = load %s\n", get_step_name(&ssa->emit, step), target);
        break;
    }
    case eOpStore: {
        ssa_store_t store = step->store;
        const char *target = ssa_operand_string(ssa, store.dst);
        const char *value = ssa_operand_string(ssa, store.src);
        write_string(io, "\t*%s = %s\n", target, value);
        break;
    }
    case eOpBinary: {
        ssa_binary_t binary = step->binary;
        const char *lhs = ssa_operand_string(ssa, binary.lhs);
        const char *rhs = ssa_operand_string(ssa, binary.rhs);
        write_string(io, "\t%%%s = %s %s, %s\n", get_step_name(&ssa->emit, step), binary_name(binary.binary), lhs, rhs);
        break;
    }
    case eOpCall: {
        ssa_call_t call = step->call;
        const char *target = ssa_operand_string(ssa, call.function);
        size_t len = typevec_len(call.args);
        write_string(io, "\t%%%s = call %s [", get_step_name(&ssa->emit, step), target);
        for (size_t i = 0; i < len; i++)
        {
            ssa_operand_t arg;
            typevec_get(call.args, i, &arg);
            const char *str = ssa_operand_string(ssa, arg);
            write_string(io, "%s", str);
            if (i < len - 1)
                write_string(io, ", ");
        }
        write_string(io, "]\n");
        break;
    }

    default: NEVER("Invalid opcode: %d", step->opcode);
    }
}

static void write_block(io_t *io, ssa_t *ssa, const ssa_block_t *block)
{
    CTASSERT(block != NULL);

    write_string(io, ".%s: [len=%zu]\n", get_block_name(&ssa->emit, block), typevec_len(block->steps));

    size_t len = typevec_len(block->steps);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_step_t *step = typevec_offset(block->steps, i);
        write_step(io, ssa, step);
    }
}

static void write_symbol_blocks(io_t *io, ssa_t *emit, const ssa_symbol_t *symbol)
{
    size_t len = vector_len(symbol->blocks);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_block_t *block = vector_get(symbol->blocks, i);
        write_block(io, emit, block);
    }
}

static void create_module_file(ssa_t *ssa, const char *root, ssa_module_t *mod)
{
    char *sourceFile = format("ssa/%s.ssa", root);

    fs_file_create(ssa->fs, sourceFile);

    io_t *src = fs_open(ssa->fs, sourceFile, eAccessWrite | eAccessText);

    char *name = str_replace(root, "/", ".");
    write_string(src, "module = %s\n", name);

    if (!map_empty(mod->globals))
    {
        write_string(src, "\n");
    }

    map_iter_t globals = map_iter(mod->globals);
    while (map_has_next(&globals))
    {
        map_entry_t entry = map_next(&globals);
        ssa_symbol_t *global = entry.value;
        write_string(src, "global %s: %s = %s\n",
            global->name,
            ssa_type_to_string(global->type),
            global->value == NULL ? "noinit" : ssa_value_to_string(global->value)
        );
        write_string(src, "\t[mangled = %s, visibility = %s, linkage = %s]\n",
            (global->linkName == NULL) ? "null" : format("\"%s\"", global->linkName),
            vis_name(global->visibility),
            link_name(global->linkage)
        );

        write_symbol_blocks(src, ssa, global);
        write_string(src, "\n");

        counter_reset(&ssa->emit);
    }

    map_iter_t functions = map_iter(mod->functions);
    while (map_has_next(&functions))
    {
        map_entry_t entry = map_next(&functions);
        ssa_symbol_t *function = entry.value;
        const char *where = (function->entry == NULL) ? "extern" : "function";
        write_string(src, "%s %s: %s\n", where, function->name, ssa_type_to_string(function->type));
        write_string(src, "\t[mangled = %s, visibility = %s, linkage = %s]\n",
            (function->linkName == NULL) ? "null" : format("\"%s\"", function->linkName),
            vis_name(function->visibility),
            link_name(function->linkage)
        );

        if (function->locals != NULL)
        {
            size_t len = typevec_len(function->locals);
            for (size_t i = 0; i < len; i++)
            {
                ssa_local_t local;
                typevec_get(function->locals, i, &local);
                write_string(src, "\t[local.%zu name=`%s` type=%s]\n", i, local.name, ssa_type_to_string(local.type));
            }
        }

        if (function->entry != NULL)
        {
            write_symbol_blocks(src, ssa, function);
        }

        counter_reset(&ssa->emit);
    }

    map_set_ptr(ssa->modules, mod, sourceFile);

    io_close(src);
}

static void create_module_dir(ssa_t *emit, const char *root, ssa_module_t *mod)
{
    const char *name = mod->name;
    CTASSERT(name != NULL);

    const char *path = (root == NULL) ? name : format("%s/%s", root, name);

    create_module_file(emit, path, mod);

    if (map_empty(mod->modules))
    {
        return;
    }

    char *sourceDir = format("ssa/%s", path);

    fs_dir_create(emit->fs, sourceDir);

    map_iter_t iter = map_iter(mod->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        create_module_dir(emit, path, entry.value);
    }
}

static void create_root_dir(ssa_t *emit, ssa_module_t *mod)
{
    map_iter_t iter = map_iter(mod->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        ssa_module_t *child = entry.value;
        create_module_dir(emit, NULL, child);
    }
}

ssa_emit_result_t emit_ssa(const emit_options_t *options)
{
    ssa_t ssa = {
        .emit = {
            .reports = options->reports,
            .blockNames = names_new(64),
            .vregNames = names_new(64),
        },
        .fs = options->fs,
        .modules = map_optimal(4),
    };

    fs_dir_create(ssa.fs, "ssa");

    create_root_dir(&ssa, options->mod);

    ssa_emit_result_t result = {
        .stub = NULL
    };

    return result;
}
#endif