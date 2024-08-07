#include "cthulhu/ssa/ssa.h"
#include "base/panic.h"
#include "base/util.h"
#include "debug/target.h"

#include "common.h"

#include "cthulhu/broker/broker.h"

#include "fs/fs.h"
#include "io/io.h"
#include "notify/notify.h"

#include "core/macros.h"

#include "std/map.h"
#include "std/set.h"
#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"

typedef struct ssa_emit_t
{
    emit_t emit;

    fs_t *fs;
    map_t *deps;
} ssa_emit_t;

static char *fn_params_to_string(typevec_t *params, arena_t *arena)
{
    size_t len = typevec_len(params);
    vector_t *vec = vector_of(len, arena);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_param_t *param = typevec_offset(params, i);
        const char *ty = type_to_string(param->type, arena);
        vector_set(vec, i, str_format(arena, "%s: %s", param->name, ty));
    }

    return str_join(", ", vec, arena);
}

static vector_t *join_attribs(const ssa_symbol_t *symbol, arena_t *arena)
{
    vector_t *attribs = vector_new(2, arena);

    if (symbol->linkage_string != NULL)
    {
        vector_push(&attribs, str_format(arena, "extern = `%s`", symbol->linkage_string));
    }

    vector_push(&attribs, str_format(arena, "linkage = %s", linkage_string(symbol->linkage)));
    vector_push(&attribs, str_format(arena, "visibility = %s", visibility_string(symbol->visibility)));

    return attribs;
}

static void emit_ssa_attribs(io_t *io, const ssa_symbol_t *symbol, arena_t *arena)
{
    io_printf(io, "\t[%s]\n", str_join(", ", join_attribs(symbol, arena), arena));
}

static const char *value_to_string(const ssa_value_t *value, arena_t *arena);

static const char *pointer_value_to_string(const ssa_value_t *value, arena_t *arena)
{
    ssa_literal_value_t literal = ssa_value_get_literal(value);
    size_t len = vector_len(literal.data);
    vector_t *parts = vector_new(16, arena);
    for (size_t i = 0; i < CT_MIN(len, 16); i++)
    {
        const ssa_value_t *elem = vector_get(literal.data, i);
        const char *it = value_to_string(elem, arena);
        vector_push(&parts, (char*)it);
    }

    if (len > 16) { vector_push(&parts, "..."); }

    char *joined = str_join(", ", parts, arena);
    return str_format(arena, "[%s]", joined);
}

static const char *value_to_string(const ssa_value_t *value, arena_t *arena)
{
    if (!value->init) { return "noinit"; }

    const ssa_type_t *type = value->type;
    switch (type->kind)
    {
    case eTypeDigit: return mpz_get_str(NULL, 10, ssa_value_get_literal(value).digit);
    case eTypeBool: return ssa_value_get_bool(value) ? "true" : "false";
    case eTypeUnit: return "unit";
    case eTypeEmpty: return "empty";
    case eTypePointer: return pointer_value_to_string(value, arena);

    default: CT_NEVER("unknown type kind %d", type->kind);
    }
}

static const char *operand_to_string(ssa_emit_t *emit, ssa_operand_t operand)
{
    emit_t *base = &emit->emit;
    arena_t *arena = base->arena;
    switch (operand.kind)
    {
    case eOperandEmpty: return "empty";
    case eOperandBlock:
        return str_format(arena, ".%s", get_block_name(&emit->emit, operand.bb));
    case eOperandImm:
        return str_format(arena, "$%s", value_to_string(operand.value, arena));
    case eOperandReg:
        return str_format(arena, "%%%s", get_step_from_block(&emit->emit, operand.vreg_context, operand.vreg_index));
    case eOperandGlobal: {
        const ssa_symbol_t *symbol = operand.global;
        return str_format(arena, "@%s", symbol->name);
    }
    case eOperandFunction: {
        const ssa_symbol_t *symbol = operand.function;
        return str_format(arena, "::%s", symbol->name);
    }
    case eOperandLocal: {
        size_t index = operand.local;
        return str_format(arena, "local(%zu)", index);
    }
    case eOperandParam: {
        size_t index = operand.param;
        return str_format(arena, "param(%zu)", index);
    }
    default: CT_NEVER("unknown operand kind %d", operand.kind);
    }
}

static void emit_ssa_block(ssa_emit_t *emit, io_t *io, const ssa_block_t *bb)
{
    emit_t *base = &emit->emit;
    size_t len = typevec_len(bb->steps);
    io_printf(io, ".%s: [len=%zu]\n", get_block_name(&emit->emit, bb), len);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_step_t *step = typevec_offset(bb->steps, i);
        switch (step->opcode)
        {
        case eOpValue:
            io_printf(io, "\t%%%s = const %s\n",
                get_step_name(&emit->emit, step),
                value_to_string(step->value, base->arena)
            );
            break;
        case eOpNop:
            io_printf(io, "\tnop\n");
            break;
        case eOpUnary: {
            ssa_unary_t unary = step->unary;
            io_printf(io, "\t%%%s = unary %s %s\n",
                get_step_name(&emit->emit, step),
                unary_name(unary.unary),
                operand_to_string(emit, unary.operand)
            );
            break;
        }
        case eOpBinary: {
            ssa_binary_t binary = step->binary;
            io_printf(io, "\t%%%s = binary %s %s %s\n",
                get_step_name(&emit->emit, step),
                binary_name(binary.binary),
                operand_to_string(emit, binary.lhs),
                operand_to_string(emit, binary.rhs)
            );
            break;
        }
        case eOpCast: {
            ssa_cast_t cast = step->cast;
            io_printf(io, "\t%%%s = cast %s %s\n",
                get_step_name(&emit->emit, step),
                type_to_string(cast.type, base->arena),
                operand_to_string(emit, cast.operand)
            );
            break;
        }
        case eOpLoad: {
            ssa_load_t load = step->load;
            io_printf(io, "\t%%%s = load %s\n",
                get_step_name(&emit->emit, step),
                operand_to_string(emit, load.src)
            );
            break;
        }
        case eOpOffset: {
            ssa_offset_t offset = step->offset;
            io_printf(io, "\t%%%s = offset %s %s\n",
                get_step_name(&emit->emit, step),
                operand_to_string(emit, offset.array),
                operand_to_string(emit, offset.offset)
            );
            break;
        }
        case eOpMember: {
            ssa_member_t member = step->member;
            io_printf(io, "\t%%%s = member %s.%zu\n",
                get_step_name(&emit->emit, step),
                operand_to_string(emit, member.object),
                member.index
            );
            break;
        }
        case eOpAddress: {
            ssa_addr_t addr = step->addr;
            io_printf(io, "\t%%%s = addr %s\n",
                get_step_name(&emit->emit, step),
                operand_to_string(emit, addr.symbol)
            );
            break;
        }
        case eOpReturn: {
            ssa_return_t ret = step->ret;
            io_printf(io, "\tret %s\n", operand_to_string(emit, ret.value));
            break;
        }
        case eOpJump: {
            ssa_jump_t jmp = step->jump;
            io_printf(io, "\tjump %s\n", operand_to_string(emit, jmp.target));
            break;
        }
        case eOpStore: {
            ssa_store_t store = step->store;
            io_printf(io, "\tstore %s %s\n",
                operand_to_string(emit, store.dst),
                operand_to_string(emit, store.src)
            );
            break;
        }
        case eOpCall: {
            ssa_call_t call = step->call;
            size_t args_len = typevec_len(call.args);
            vector_t *args = vector_of(args_len, base->arena);
            for (size_t arg_idx = 0; arg_idx < args_len; arg_idx++)
            {
                const ssa_operand_t *arg = typevec_offset(call.args, arg_idx);
                vector_set(args, arg_idx, (char*)operand_to_string(emit, *arg));
            }
            io_printf(io, "\t%%%s = call %s (%s)\n",
                get_step_name(&emit->emit, step),
                operand_to_string(emit, call.function),
                str_join(", ", args, base->arena)
            );
            break;
        }
        case eOpBranch: {
            ssa_branch_t branch = step->branch;
            io_printf(io, "\tbranch %s %s %s\n",
                operand_to_string(emit, branch.cond),
                operand_to_string(emit, branch.then),
                operand_to_string(emit, branch.other)
            );
            break;
        }
        case eOpCompare: {
            ssa_compare_t compare = step->compare;
            io_printf(io, "\t%%%s = compare %s %s %s\n",
                get_step_name(&emit->emit, step),
                compare_name(compare.compare),
                operand_to_string(emit, compare.lhs),
                operand_to_string(emit, compare.rhs)
            );
            break;
        }
        case eOpSizeOf: {
            ssa_sizeof_t size_of = step->size_of;
            io_printf(io, "\t%%%s = sizeof %s\n",
                get_step_name(&emit->emit, step),
                type_to_string(size_of.type, base->arena)
            );
            break;
        }
        case eOpAlignOf: {
            ssa_alignof_t align_of = step->align_of;
            io_printf(io, "\t%%%s = alignof %s\n",
                get_step_name(&emit->emit, step),
                type_to_string(align_of.type, base->arena)
            );
            break;
        }
        case eOpOffsetOf: {
            ssa_offsetof_t offset_of = step->offset_of;
            io_printf(io, "\t%%%s = offsetof %s %zu\n",
                get_step_name(&emit->emit, step),
                type_to_string(offset_of.type, base->arena),
                offset_of.index
            );
            break;
        }
        default: CT_NEVER("unknown opcode %d", step->opcode);
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

static const char *storage_to_string(ssa_storage_t storage, arena_t *arena)
{
    const char *ty = type_to_string(storage.type, arena);
    const char *quals = quals_string(storage.quals);

    return str_format(arena, "{type = %s, quals = %s, size = %zu}", ty, quals, storage.size);
}

static void emit_ssa_locals(ssa_emit_t *emit, io_t *io, typevec_t *locals)
{
    emit_t *base = &emit->emit;
    size_t len = typevec_len(locals);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_local_t *local = typevec_offset(locals, i);
        io_printf(io, "\tlocal[%zu] %s %s\n", i, type_to_string(local->type, base->arena), storage_to_string(local->storage, base->arena));
    }
}

static void emit_symbol_deps(io_t *io, const ssa_symbol_t *symbol, map_t *deps)
{
    set_t *all = map_get(deps, symbol);
    if (all != NULL)
    {
        io_printf(io, "deps: (");
        set_iter_t iter = set_iter(all);
        while (set_has_next(&iter))
        {
            const ssa_symbol_t *dep = set_next(&iter);
            io_printf(io, "%s", dep->name);

            if (set_has_next(&iter)) { io_printf(io, ", "); }
        }
        io_printf(io, ")\n");
    }
}

static void debug_emit_module(ssa_emit_t *emit, const ssa_module_t *mod)
{
    fs_t *fs = emit->fs;
    emit_t *base = &emit->emit;
    char *path = begin_module(&emit->emit, fs, mod);

    char *file = str_format(base->arena, "%s/%s.ssa", path, mod->name);
    fs_file_create(fs, file);
    vector_push(&base->files, file);

    io_t *io = fs_open(fs, file, eOsAccessWrite | eOsAccessTruncate);
    io_printf(io, "module {name=%s", mod->name);
    if (ctu_strlen(path) > 0) { io_printf(io, ", path=%s", path); }
    io_printf(io, "}\n");

    io_printf(io, "\n");

    size_t len = vector_len(mod->globals);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_symbol_t *global = vector_get(mod->globals, i);
        emit_symbol_deps(io, global, emit->deps);

        io_printf(io, "global %s: %s\n", global->name, type_to_string(global->type, base->arena));
        emit_ssa_attribs(io, global, base->arena);

        emit_ssa_blocks(emit, io, global->blocks);

        if (len >= i) { io_printf(io, "\n"); }
    }

    size_t fns = vector_len(mod->functions);
    for (size_t i = 0; i < fns; i++)
    {
        const ssa_symbol_t *fn = vector_get(mod->functions, i);
        emit_symbol_deps(io, fn, emit->deps);

        const ssa_type_t *type = fn->type;
        CTASSERTF(type->kind == eTypeClosure, "fn %s is not a closure", fn->name);
        ssa_type_closure_t closure = type->closure;

        io_printf(io, "fn %s(%s) -> %s [variadic: %s]\n",
            fn->name,
            fn_params_to_string(closure.params, base->arena), type_to_string(closure.result, base->arena),
            closure.variadic ? "true" : "false"
        );
        emit_ssa_attribs(io, fn, base->arena);

        if (fn->linkage != eLinkImport)
        {
            emit_ssa_locals(emit, io, fn->locals);
            emit_ssa_blocks(emit, io, fn->blocks);
        }

        if (len >= i) { io_printf(io, "\n"); }
    }
}

emit_result_t debug_ssa(target_runtime_t *runtime, const ssa_result_t *ssa, target_emit_t *target)
{
    arena_t *arena = runtime->arena;
    ssa_emit_t emit = {
        .emit = {
            .arena = arena,
            .reports = runtime->logger,
            .block_names = names_new(64, arena),
            .vreg_names = names_new(64, arena),
            .files = vector_new(16, arena),
        },
        .fs = target->fs,
        .deps = ssa->deps,
    };

    size_t len = vector_len(ssa->modules);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(ssa->modules, i);
        debug_emit_module(&emit, mod);
    }

    emit_result_t result = {
        .files = emit.emit.files
    };

    return result;
}
