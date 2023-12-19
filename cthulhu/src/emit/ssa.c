#include "common.h"

#include "cthulhu/util/util.h"

#include "std/map.h"
#include "std/str.h"
#include "std/vector.h"
#include "std/set.h"

#include "std/typed/vector.h"

#include "base/panic.h"

#include "core/macros.h"

#include "fs/fs.h"

#include <string.h>

typedef struct ssa_emit_t
{
    emit_t emit;

    fs_t *fs;
    map_t *deps;
} ssa_emit_t;

static char *params_to_string(typevec_t *params)
{
    size_t len = typevec_len(params);
    vector_t *vec = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_param_t *param = typevec_offset(params, i);
        const char *ty = type_to_string(param->type);
        vector_set(vec, i, format("%s: %s", param->name, ty));
    }

    return str_join(", ", vec);
}

static vector_t *join_attribs(const ssa_symbol_t *symbol)
{
    vector_t *attribs = vector_new(2);

    if (symbol->link_name != NULL)
    {
        vector_push(&attribs, format("extern = `%s`", symbol->link_name));
    }

    vector_push(&attribs, format("linkage = %s", link_name(symbol->linkage)));
    vector_push(&attribs, format("visibility = %s", vis_name(symbol->visibility)));

    return attribs;
}

static void emit_ssa_attribs(io_t *io, const ssa_symbol_t *symbol)
{
    write_string(io, "\t[%s]\n", str_join(", ", join_attribs(symbol)));
}

static const char *value_to_string(const ssa_value_t *value);

static const char *pointer_value_to_string(const ssa_value_t *value)
{
    size_t len = vector_len(value->data);
    vector_t *parts = vector_new(16);
    for (size_t i = 0; i < MIN(len, 16); i++)
    {
        const ssa_value_t *elem = vector_get(value->data, i);
        const char *it = value_to_string(elem);
        vector_push(&parts, (char*)it);
    }

    if (len > 16) { vector_push(&parts, "..."); }

    return format("[%s]", str_join(", ", parts));
}

static const char *value_to_string(const ssa_value_t *value)
{
    if (!value->init) { return "noinit"; }

    const ssa_type_t *type = value->type;
    switch (type->kind)
    {
    case eTypeDigit: return mpz_get_str(NULL, 10, value->digitValue);
    case eTypeBool: return value->boolValue ? "true" : "false";
    case eTypeUnit: return "unit";
    case eTypeEmpty: return "empty";
    case eTypePointer: return pointer_value_to_string(value);

    default: NEVER("unknown type kind %d", type->kind);
    }
}

static const char *operand_to_string(ssa_emit_t *emit, ssa_operand_t operand)
{
    switch (operand.kind)
    {
    case eOperandEmpty: return "empty";
    case eOperandBlock:
        return format(".%s", get_block_name(&emit->emit, operand.bb));
    case eOperandImm:
        return format("$%s", value_to_string(operand.value));
    case eOperandReg:
        return format("%%%s", get_step_from_block(&emit->emit, operand.vregContext, operand.vregIndex));
    case eOperandGlobal: {
        const ssa_symbol_t *symbol = operand.global;
        return format("@%s", symbol->name);
    }
    case eOperandFunction: {
        const ssa_symbol_t *symbol = operand.function;
        return format("::%s", symbol->name);
    }
    case eOperandLocal: {
        size_t index = operand.local;
        return format("local(%zu)", index);
    }
    case eOperandParam: {
        size_t index = operand.param;
        return format("param(%zu)", index);
    }
    case eOperandConst: {
        size_t index = operand.constant;
        return format("const(%zu)", index);
    }
    default: NEVER("unknown operand kind %d", operand.kind);
    }
}

static void emit_ssa_block(ssa_emit_t *emit, io_t *io, const ssa_block_t *bb)
{
    size_t len = typevec_len(bb->steps);
    write_string(io, ".%s: [len=%zu]\n", get_block_name(&emit->emit, bb), len);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_step_t *step = typevec_offset(bb->steps, i);
        switch (step->opcode)
        {
        case eOpUnary: {
            ssa_unary_t unary = step->unary;
            write_string(io, "\t%%%s = unary %s %s\n",
                get_step_name(&emit->emit, step),
                unary_name(unary.unary),
                operand_to_string(emit, unary.operand)
            );
            break;
        }
        case eOpBinary: {
            ssa_binary_t binary = step->binary;
            write_string(io, "\t%%%s = binary %s %s %s\n",
                get_step_name(&emit->emit, step),
                binary_name(binary.binary),
                operand_to_string(emit, binary.lhs),
                operand_to_string(emit, binary.rhs)
            );
            break;
        }
        case eOpCast: {
            ssa_cast_t cast = step->cast;
            write_string(io, "\t%%%s = cast %s %s\n",
                get_step_name(&emit->emit, step),
                type_to_string(cast.type),
                operand_to_string(emit, cast.operand)
            );
            break;
        }
        case eOpLoad: {
            ssa_load_t load = step->load;
            write_string(io, "\t%%%s = load %s\n",
                get_step_name(&emit->emit, step),
                operand_to_string(emit, load.src)
            );
            break;
        }
        case eOpOffset: {
            ssa_offset_t offset = step->offset;
            write_string(io, "\t%%%s = offset %s %s\n",
                get_step_name(&emit->emit, step),
                operand_to_string(emit, offset.array),
                operand_to_string(emit, offset.offset)
            );
            break;
        }
        case eOpMember: {
            ssa_member_t member = step->member;
            write_string(io, "\t%%%s = member %s.%zu\n",
                get_step_name(&emit->emit, step),
                operand_to_string(emit, member.object),
                member.index
            );
            break;
        }
        case eOpAddress: {
            ssa_addr_t addr = step->addr;
            write_string(io, "\t%%%s = addr %s\n",
                get_step_name(&emit->emit, step),
                operand_to_string(emit, addr.symbol)
            );
            break;
        }
        case eOpReturn: {
            ssa_return_t ret = step->ret;
            write_string(io, "\tret %s\n", operand_to_string(emit, ret.value));
            break;
        }
        case eOpJump: {
            ssa_jump_t jmp = step->jump;
            write_string(io, "\tjump %s\n", operand_to_string(emit, jmp.target));
            break;
        }
        case eOpStore: {
            ssa_store_t store = step->store;
            write_string(io, "\tstore %s %s\n",
                operand_to_string(emit, store.dst),
                operand_to_string(emit, store.src)
            );
            break;
        }
        case eOpCall: {
            ssa_call_t call = step->call;
            size_t args_len = typevec_len(call.args);
            vector_t *args = vector_of(args_len);
            for (size_t arg_idx = 0; arg_idx < args_len; arg_idx++)
            {
                const ssa_operand_t *arg = typevec_offset(call.args, arg_idx);
                vector_set(args, arg_idx, (char*)operand_to_string(emit, *arg));
            }
            write_string(io, "\t%%%s = call %s (%s)\n",
                get_step_name(&emit->emit, step),
                operand_to_string(emit, call.function),
                str_join(", ", args)
            );
            break;
        }
        case eOpBranch: {
            ssa_branch_t branch = step->branch;
            write_string(io, "\tbranch %s %s %s\n",
                operand_to_string(emit, branch.cond),
                operand_to_string(emit, branch.then),
                operand_to_string(emit, branch.other)
            );
            break;
        }
        case eOpCompare: {
            ssa_compare_t compare = step->compare;
            write_string(io, "\t%%%s = compare %s %s %s\n",
                get_step_name(&emit->emit, step),
                compare_name(compare.compare),
                operand_to_string(emit, compare.lhs),
                operand_to_string(emit, compare.rhs)
            );
            break;
        }
        default: NEVER("unknown opcode %d", step->opcode);
        }
    }
}

static void emit_ssa_blocks(ssa_emit_t *emit, io_t *io, vector_t *bbs)
{
    size_t len = vector_len(bbs);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_block_t *bb = vector_get(bbs, i);
        emit_ssa_block(emit, io, bb);
    }
}

static void emit_ssa_consts(ssa_emit_t *emit, io_t *io, vector_t *consts)
{
    CTU_UNUSED(emit);
    size_t len = vector_len(consts);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_value_t *value = vector_get(consts, i);
        write_string(io, "\tconst[%zu] %s = %s\n", i, type_to_string(value->type), value_to_string(value));
    }
}

static const char *storage_to_string(ssa_storage_t storage)
{
    const char *ty = type_to_string(storage.type);
    const char *quals = quals_name(storage.quals);

    return format("{type = %s, quals = %s, size = %zu}", ty, quals, storage.size);
}

static void emit_ssa_locals(ssa_emit_t *emit, io_t *io, typevec_t *locals)
{
    CTU_UNUSED(emit);
    size_t len = typevec_len(locals);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_local_t *local = typevec_offset(locals, i);
        write_string(io, "\tlocal[%zu] %s %s\n", i, type_to_string(local->type), storage_to_string(local->storage));
    }
}

static void emit_symbol_deps(io_t *io, const ssa_symbol_t *symbol, map_t *deps)
{
    set_t *all = map_get_ptr(deps, symbol);
    if (all != NULL)
    {
        write_string(io, "deps: (");
        set_iter_t iter = set_iter(all);
        while (set_has_next(&iter))
        {
            const ssa_symbol_t *dep = set_next(&iter);
            write_string(io, "%s", dep->name);

            if (set_has_next(&iter)) { write_string(io, ", "); }
        }
        write_string(io, ")\n");
    }
}

static void emit_ssa_module(ssa_emit_t *emit, const ssa_module_t *mod)
{
    fs_t *fs = emit->fs;
    char *path = begin_module(&emit->emit, fs, mod);

    char *file = format("%s/%s.ssa", path, mod->name);
    fs_file_create(fs, file);

    io_t *io = fs_open(fs, file, eAccessWrite | eAccessText);
    write_string(io, "module {name=%s", mod->name);
    if (strlen(path) > 0) { write_string(io, ", path=%s", path); }
    write_string(io, "}\n");

    write_string(io, "\n");

    size_t len = vector_len(mod->globals);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_symbol_t *global = vector_get(mod->globals, i);
        emit_symbol_deps(io, global, emit->deps);

        write_string(io, "global %s: %s\n", global->name, type_to_string(global->type));
        emit_ssa_attribs(io, global);

        emit_ssa_consts(emit, io, global->consts);
        emit_ssa_blocks(emit, io, global->blocks);

        if (len >= i) { write_string(io, "\n"); }
    }

    size_t fns = vector_len(mod->functions);
    for (size_t i = 0; i < fns; i++)
    {
        const ssa_symbol_t *fn = vector_get(mod->functions, i);
        emit_symbol_deps(io, fn, emit->deps);

        const ssa_type_t *type = fn->type;
        CTASSERTF(type->kind == eTypeClosure, "fn %s is not a closure", fn->name);
        ssa_type_closure_t closure = type->closure;

        write_string(io, "fn %s(%s) -> %s [variadic: %s]\n",
            fn->name,
            params_to_string(closure.params), type_to_string(closure.result),
            closure.variadic ? "true" : "false"
        );
        emit_ssa_attribs(io, fn);

        if (fn->linkage != eLinkImport)
        {
            emit_ssa_consts(emit, io, fn->consts);
            emit_ssa_locals(emit, io, fn->locals);
            emit_ssa_blocks(emit, io, fn->blocks);
        }

        if (len >= i) { write_string(io, "\n"); }
    }
}

ssa_emit_result_t emit_ssa(const ssa_emit_options_t *options)
{
    const emit_options_t opts = options->opts;
    ssa_emit_t emit = {
        .emit = {
            .reports = opts.reports,
            .block_names = names_new(64),
            .vreg_names = names_new(64),
        },
        .fs = opts.fs,
        .deps = opts.deps
    };

    size_t len = vector_len(opts.modules);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(opts.modules, i);
        emit_ssa_module(&emit, mod);
    }

    ssa_emit_result_t result = { NULL };
    return result;
}
