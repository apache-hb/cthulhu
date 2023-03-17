#include "cthulhu/ssa/ssa.h"

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/query.h"

#include "base/macros.h"
#include "base/memory.h"
#include "base/util.h"
#include "base/panic.h"

#include "std/map.h"
#include "std/vector.h"
#include "std/str.h"
#include "std/set.h"

#include "report/report.h"

#include "common.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    reports_t *reports;

    map_t *aggregates;

    map_t *globals;
    map_t *functions;

    set_t *strings;

    map_t *importedSymbols; // map_t<const char *, const hlir_t *>

    map_t *currentLocals; // map_t<const hlir_t *, size_t> // map of local variables to their index in the locals vector
    map_t *currentParams; // map_t<const hlir_t *, size_t> // map of parameters to their index in the params vector

    ssa_block_t *currentBlock;
    size_t stepIdx;
    size_t blockIdx;
} ssa_t;

static ssa_block_t *block_new(const char *id) 
{
    ssa_block_t *block = ctu_malloc(sizeof(ssa_block_t));
    block->steps = vector_new(16);
    block->id = id;
    return block;
}

static ssa_block_t *block_gen(ssa_t *ssa, const char *id)
{
    return block_new(format("%s%zu", id, ssa->blockIdx++));
}

static ssa_kind_t get_type_kind(ssa_t *ssa, const hlir_t *hlir)
{
    hlir_kind_t kind = get_hlir_kind(hlir);
    switch (kind)
    {
    case eHlirDigit: return eTypeDigit;
    case eHlirDecimal: return eTypeDecimal;
    case eHlirBool: return eTypeBool;
    case eHlirString: return eTypeString;
    case eHlirPointer: return eTypePointer;
    case eHlirUnit: return eTypeUnit;
    case eHlirEmpty: return eTypeEmpty;

    case eHlirClosure:
    case eHlirFunction: 
        return eTypeSignature;

    case eHlirOpaque: return eTypeOpaque;
    case eHlirStruct: return eTypeStruct;
    case eHlirArray: return eTypeArray;

    default:
        report(ssa->reports, eInternal, get_hlir_node(hlir), "no respective ssa type for %s", hlir_kind_to_string(kind));
        return eTypeEmpty;
    }
}

typedef struct ssa_type_result_t {
    ssa_type_t *type;
    bool complete;
} ssa_type_result_t;

static ssa_type_result_t ssa_anytype_new(ssa_t *ssa, const void *key, const char *name, ssa_kind_t kind)
{
    if (key != NULL)
    {
        ssa_type_t *existing = map_get_ptr(ssa->aggregates, key);
        if (existing != NULL)
        {
            ssa_type_result_t result = { existing, true };
            return result;
        }
    }

    ssa_type_t *it = ctu_malloc(sizeof(ssa_type_t));
    it->kind = kind;
    it->name = name;

    if (kind == eTypeStruct || kind == eTypePointer || kind == eTypeSignature)
    {
        CTASSERT(key != NULL);
        map_set_ptr(ssa->aggregates, key, it);
    }
    
    ssa_type_result_t result = { it, false };

    return result;
}

static ssa_type_t *type_new(ssa_t *ssa, const hlir_t *type)
{
    const hlir_t *real = hlir_follow_type(type);
    hlir_kind_t kind = get_hlir_kind(real);
    ssa_type_result_t result = ssa_anytype_new(ssa, real, get_hlir_name(real), get_type_kind(ssa, real));
    if (result.complete)
    {
        return result.type;
    }

    ssa_type_t *it = result.type;

    switch (kind) 
    {
    case eHlirDigit:
        it->digit = real->width;
        it->sign = real->sign;
        break;

    case eHlirFunction:
    case eHlirClosure: {
        it->result = type_new(ssa, real->result);
        it->variadic = real->variadic;
        it->args = vector_of(vector_len(real->params));
        for (size_t i = 0; i < vector_len(real->params); i++) 
        {
            const char *name = get_hlir_name(vector_get(real->params, i));
            ssa_type_t *arg = type_new(ssa, vector_get(real->params, i));
            vector_set(it->args, i, ssa_param_new(name, arg));
        }
        break;
    }

    case eHlirStruct: {
        size_t len = vector_len(real->fields);
        it->fields = vector_of(len);
        for (size_t i = 0; i < len; i++) 
        {
            const hlir_t *field = vector_get(real->fields, i);
            const char *name = get_hlir_name(field);
            ssa_type_t *type = type_new(ssa, get_hlir_type(field));
            vector_set(it->fields, i, ssa_param_new(name, type));
        }
        break;
    }

    case eHlirPointer:
        it->ptr = type_new(ssa, real->ptr);
        break;

    case eHlirDecimal:
    case eHlirOpaque:
    case eHlirBool:
    case eHlirUnit:
    case eHlirEmpty:
    case eHlirString: 
        break;

    default:
        report(ssa->reports, eInternal, get_hlir_node(real), "no respective ssa type for %s", hlir_kind_to_string(kind));
        break;
    }

    return it;
}

/**
 * @brief add a step to the current block and return the register it was assigned to
 */
static ssa_operand_t add_step(ssa_t *ssa, ssa_step_t step)
{
    ssa_step_t *ptr = BOX(step);
    if (ptr->id == NULL)
    {
        ptr->id = format("%zu", ssa->stepIdx++);
    }
    vector_push(&ssa->currentBlock->steps, ptr);
    ssa_operand_t op = {
        .kind = eOperandReg,
        .vreg = ptr
    };
    return op;
}

static bool is_imported(linkage_t link)
{
    return link == eLinkImported;
}

static ssa_flow_t *flow_new(ssa_t *ssa, const hlir_t *symbol, ssa_type_t *type)
{
    UNUSED(ssa);

    ssa_flow_t *flow = ctu_malloc(sizeof(ssa_flow_t));
    flow->name = get_hlir_name(symbol);
    flow->type = type;

    const hlir_attributes_t *attribs = get_hlir_attributes(symbol);

    if (is_imported(attribs->linkage))
    {
        flow->symbol = attribs->mangle;
    }
    else
    {
        flow->entry = NULL;
    }

    // TODO: a little iffy
    flow->linkage = attribs->linkage;
    flow->visibility = attribs->visibility;

    return flow;
}

static ssa_flow_t *global_new(ssa_t *ssa, const hlir_t *global)
{
    ssa_type_t *type = type_new(ssa, get_hlir_type(global));
    ssa_flow_t *flow = flow_new(ssa, global, type);
    
    if (!is_imported(flow->linkage))
    {
        flow->value = NULL;
    }

    return flow;
}

static ssa_flow_t *function_new(ssa_t *ssa, const hlir_t *function)
{
    ssa_type_t *type = type_new(ssa, get_hlir_type(function));
    ssa_flow_t *flow = flow_new(ssa, function, type);

    if (!is_imported(flow->linkage))
    {
        flow->locals = vector_new(0);
    }

    return flow;
}

static void fwd_global(ssa_t *ssa, const hlir_t *global)
{
    ssa_flow_t *flow = global_new(ssa, global);
    map_set_ptr(ssa->globals, global, flow);
}

static void fwd_function(ssa_t *ssa, const hlir_t *function)
{
    ssa_flow_t *flow = function_new(ssa, function);
    map_set_ptr(ssa->functions, function, flow);
}

static ssa_operand_t compile_rvalue(ssa_t *ssa, const hlir_t *hlir);
static ssa_operand_t compile_stmt(ssa_t *ssa, const hlir_t *stmt);

static ssa_type_t *new_digit_type(ssa_t *ssa, digit_t width, sign_t sign, const char *name)
{
    ssa_type_result_t result = ssa_anytype_new(ssa, NULL, name, eTypeDigit);
    ssa_type_t *it = result.type;
    it->digit = width;
    it->sign = sign;
    return it;
}

static ssa_type_t *ssa_get_digit_type(ssa_t *ssa, const hlir_t *digit)
{
    CTASSERTF(hlir_is(digit, eHlirDigitLiteral), "expected digit literal, got %s", hlir_kind_to_string(get_hlir_kind(digit)));

    const hlir_t *type = hlir_follow_type(get_hlir_type(digit));

    CTASSERT(hlir_is(type, eHlirDigit));

    return new_digit_type(ssa, type->width, type->sign, get_hlir_name(type));
}

static ssa_type_t *ssa_get_decimal_type(ssa_t *ssa, const hlir_t *decimal)
{
    CTASSERTF(hlir_is(decimal, eHlirDecimalLiteral), "expected decimal literal, got %s", hlir_kind_to_string(get_hlir_kind(decimal)));

    const hlir_t *type = hlir_follow_type(get_hlir_type(decimal));

    CTASSERT(hlir_is(type, eHlirDecimal));

    return ssa_anytype_new(ssa, NULL, "float", eTypeDecimal).type;
}

static ssa_type_t *ssa_get_bool_type(ssa_t *ssa)
{
    return ssa_anytype_new(ssa, NULL, "bool", eTypeBool).type;
}

static ssa_type_t *ssa_get_string_type(ssa_t *ssa)
{
    return ssa_anytype_new(ssa, NULL, "string", eTypeString).type;
}

static ssa_value_t *ssa_value_digit_new(ssa_t *ssa, const hlir_t *hlir)
{
    ssa_value_t *it = ssa_value_new(ssa_get_digit_type(ssa, hlir), true);

    memcpy(it->digit, hlir->digit, sizeof(mpz_t));

    return it;
}

static ssa_value_t *ssa_value_decimal_new(ssa_t *ssa, const hlir_t *hlir)
{
    ssa_value_t *it = ssa_value_new(ssa_get_decimal_type(ssa, hlir), true);

    memcpy(it->decimal, hlir->decimal, sizeof(mpq_t));

    return it;
}

static ssa_value_t *value_bool_new(ssa_t *ssa, const hlir_t *hlir)
{
    ssa_value_t *it = ssa_value_new(ssa_get_bool_type(ssa), true);
    it->boolean = hlir->boolean;

    return it;
}

static ssa_value_t *value_string_new(ssa_t *ssa, const hlir_t *hlir)
{
    ssa_value_t *it = ssa_value_new(ssa_get_string_type(ssa), true);
    it->string = hlir->stringLiteral;

    return it;
}

static ssa_operand_t compile_digit(ssa_t *ssa, const hlir_t *hlir)
{
    ssa_operand_t op = {
        .kind = eOperandImm,
        .value = ssa_value_digit_new(ssa, hlir)
    };

    return op;
}

static ssa_operand_t compile_decimal(ssa_t *ssa, const hlir_t *hlir)
{
    ssa_operand_t op = {
        .kind = eOperandImm,
        .value = ssa_value_decimal_new(ssa, hlir)
    };

    return op;
}

static ssa_operand_t compile_string(ssa_t *ssa, const hlir_t *hlir)
{
    // TODO: string
    ssa_operand_t op = {
        .kind = eOperandImm,
        .value = value_string_new(ssa, hlir)
    };

    return op;
}

static ssa_operand_t compile_bool(ssa_t *ssa, const hlir_t *hlir)
{
    ssa_operand_t op = {
        .kind = eOperandImm,
        .value = value_bool_new(ssa, hlir)
    };

    return op;
}

static ssa_operand_t compile_binary(ssa_t *ssa, const hlir_t *hlir)
{
    ssa_operand_t lhs = compile_rvalue(ssa, hlir->lhs);
    ssa_operand_t rhs = compile_rvalue(ssa, hlir->rhs);

    ssa_step_t step = {
        .opcode = eOpBinary,
        .type = type_new(ssa, get_hlir_type(hlir)),
        .binary = {
            .op = hlir->binary,
            .lhs = lhs,
            .rhs = rhs
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t compile_load(ssa_t *ssa, const hlir_t *hlir)
{
    ssa_operand_t name = compile_rvalue(ssa, hlir->read);

    ssa_step_t step = {
        .opcode = eOpLoad,
        .type = type_new(ssa, get_hlir_type(hlir->read)),
        .load = {
            .src = name
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t compile_global_ref(ssa_t *ssa, const hlir_t *hlir)
{
    const ssa_flow_t *ref = map_get_ptr(ssa->globals, hlir);

    ssa_operand_t op = {
        .kind = eOperandGlobal,
        .global = ref
    };

    return op;
}

static ssa_operand_t compile_local_ref(ssa_t *ssa, const hlir_t *hlir)
{
    size_t index = (uintptr_t)map_get_ptr(ssa->currentLocals, hlir);

    ssa_operand_t op = {
        .kind = eOperandLocal,
        .local = index
    };

    return op;
}

static ssa_operand_t compile_cast(ssa_t *ssa, const hlir_t *hlir)
{
    ssa_operand_t op = compile_rvalue(ssa, hlir->expr);
    const ssa_type_t *type = type_new(ssa, get_hlir_type(hlir));

    ssa_step_t step = {
        .opcode = eOpCast,
        .type = type,
        .cast = {
            .op = hlir->cast,
            .operand = op,

            .type = type
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t compile_call(ssa_t *ssa, const hlir_t *hlir)
{
    size_t len = vector_len(hlir->args);
    ssa_operand_t *args = ctu_malloc(sizeof(ssa_operand_t) * MAX(len, 1));
    for (size_t i = 0; i < len; i++)
    {
        const hlir_t *arg = vector_get(hlir->args, i);
        args[i] = compile_rvalue(ssa, arg);
    }

    ssa_operand_t func = compile_rvalue(ssa, hlir->call);

    ssa_step_t step = {
        .opcode = eOpCall,
        .type = type_new(ssa, get_hlir_type(hlir)),
        .call = {
            .args = args,
            .len = len,
            .symbol = func
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t compile_name(ssa_t *ssa, const hlir_t *hlir)
{
    const ssa_flow_t *ref = map_get_ptr(ssa->functions, hlir);
    CTASSERT(ref != NULL);

    ssa_operand_t op = {
        .kind = eOperandFunction,
        .function = ref
    };

    return op;
}

static ssa_operand_t compile_lvalue(ssa_t *ssa, const hlir_t *hlir);

static ssa_operand_t compile_global_lvalue(ssa_t *ssa, const hlir_t *hlir)
{
    const ssa_flow_t *ref = map_get_ptr(ssa->globals, hlir);
    CTASSERT(ref != NULL);

    ssa_operand_t op = {
        .kind = eOperandGlobal,
        .global = ref
    };

    return op;
}

static ssa_operand_t compile_unary(ssa_t *ssa, const hlir_t *hlir)
{
    ssa_operand_t op = compile_rvalue(ssa, hlir->expr);

    ssa_step_t step = {
        .opcode = eOpUnary,
        .type = type_new(ssa, get_hlir_type(hlir)),
        .unary = {
            .op = hlir->unary,
            .operand = op
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t compile_local_lvalue(ssa_t *ssa, const hlir_t *hlir)
{
    const size_t idx = (size_t)(uintptr_t)map_get_ptr(ssa->currentLocals, hlir);

    ssa_operand_t op = {
        .kind = eOperandLocal,
        .local = idx
    };

    return op;
}

static ssa_operand_t *make_offset(const hlir_t *object, const hlir_t *member)
{
    const hlir_t *type = get_hlir_type(object);
    bool indirect = false;
    if (hlir_is(type, eHlirPointer))
    {
        indirect = true;
        type = type->ptr;
    }

    CTASSERT(hlir_is(type, eHlirStruct));

    size_t field = vector_find(type->fields, member);

    CTASSERT(field != SIZE_MAX);

    ssa_operand_t offset = {
        .kind = eOperandOffset,
        .indirect = indirect,
        .index = field
    };

    return BOX(offset);
}

static ssa_operand_t compile_access_lvalue(ssa_t *ssa, const hlir_t *hlir)
{
    ssa_operand_t read = compile_lvalue(ssa, hlir->object);

    read.offset = make_offset(hlir->object, hlir->member);

    return read;
}

static ssa_operand_t operand_bb(const ssa_block_t *dst)
{
    ssa_operand_t op = {
        .kind = eOperandBlock,
        .bb = dst
    };

    return op;
}

static ssa_operand_t operand_empty(void)
{
    ssa_operand_t op = {
        .kind = eOperandEmpty
    };

    return op;
}

static ssa_operand_t compile_param(ssa_t *ssa, const hlir_t *hlir)
{
    const void *ref = map_get_ptr(ssa->currentParams, hlir);

    ssa_operand_t op = {
        .kind = eOperandParam,
        .param = (uintptr_t)ref
    };

    return op;
}

static ssa_operand_t compile_lvalue(ssa_t *ssa, const hlir_t *hlir)
{
    hlir_kind_t kind = get_hlir_kind(hlir);
    switch (kind)
    {
    case eHlirGlobal:
        return compile_global_lvalue(ssa, hlir);

    case eHlirLocal:
        return compile_local_lvalue(ssa, hlir);

    case eHlirParam:
        return compile_param(ssa, hlir);

    case eHlirAccess:
        return compile_access_lvalue(ssa, hlir);

    default:
        report(ssa->reports, eInternal, get_hlir_node(hlir), "compile-lvalue %s", hlir_kind_to_string(kind));
        return operand_empty();
    }
}

static ssa_operand_t compile_rvalue_access(ssa_t *ssa, const hlir_t *hlir)
{
    ssa_operand_t read = compile_lvalue(ssa, hlir->object);

    read.offset = make_offset(hlir->object, hlir->member);

    return read;
}

static ssa_operand_t compile_addr(ssa_t *ssa, const hlir_t *hlir)
{
    ssa_operand_t op = compile_lvalue(ssa, hlir->expr);

    ssa_step_t step = {
        .opcode = eOpAddr,
        .type = type_new(ssa, get_hlir_type(hlir)),
        .addr = {
            .expr = op
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t compile_sizeof(ssa_t *ssa, const hlir_t *hlir)
{
    ssa_type_t *type = type_new(ssa, hlir->operand);

    ssa_step_t step = {
        .opcode = eOpSizeOf,
        .type = new_digit_type(ssa, eDigitSize, eUnsigned, "size"),
        .size = {
            .type = type
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t compile_builtin(ssa_t *ssa, const hlir_t *hlir)
{
    switch (hlir->builtin)
    {
    case eBuiltinSizeOf:
        return compile_sizeof(ssa, hlir);
    
    default:
        report(ssa->reports, eInternal, get_hlir_node(hlir), "compile-builtin %d", hlir->builtin);
        return operand_empty();
    }
}

static ssa_operand_t compile_rvalue(ssa_t *ssa, const hlir_t *hlir)
{
    hlir_kind_t kind = get_hlir_kind(hlir);
    switch (kind)
    {
    case eHlirDigitLiteral:
        return compile_digit(ssa, hlir);

    case eHlirDecimalLiteral:
        return compile_decimal(ssa, hlir);

    case eHlirStringLiteral:
        return compile_string(ssa, hlir);

    case eHlirBoolLiteral:
        return compile_bool(ssa, hlir);

    case eHlirUnary:
        return compile_unary(ssa, hlir);

    case eHlirBinary:
        return compile_binary(ssa, hlir);

    case eHlirLoad:
        return compile_load(ssa, hlir);

    case eHlirGlobal:
        return compile_global_ref(ssa, hlir);

    case eHlirLocal:
        return compile_local_ref(ssa, hlir);

    case eHlirCast:
        return compile_cast(ssa, hlir);

    case eHlirCall:
        return compile_call(ssa, hlir);

    case eHlirFunction:
        return compile_name(ssa, hlir);

    case eHlirParam:
        return compile_param(ssa, hlir);

    case eHlirAccess:
        return compile_rvalue_access(ssa, hlir);

    case eHlirAddr:
        return compile_addr(ssa, hlir);

    case eHlirBuiltin:
        return compile_builtin(ssa, hlir);

    default:
        report(ssa->reports, eInternal, get_hlir_node(hlir), "compile-rvalue %s", hlir_kind_to_string(kind));
        return operand_empty();
    }
}

static ssa_operand_t compile_stmts(ssa_t *ssa, const hlir_t *stmt)
{
    for (size_t i = 0; i < vector_len(stmt->stmts); i++)
    {
        compile_stmt(ssa, vector_get(stmt->stmts, i));
    }

    return operand_bb(ssa->currentBlock);
}

static ssa_operand_t compile_assign(ssa_t *ssa, const hlir_t *stmt)
{
    ssa_operand_t dst = compile_lvalue(ssa, stmt->dst);
    ssa_operand_t src = compile_rvalue(ssa, stmt->src);

    ssa_step_t step = {
        .opcode = eOpStore,
        .type = type_empty_new("empty"),
        .store = {
            .dst = dst,
            .src = src
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t compile_compare_compare(ssa_t *ssa, const hlir_t *cond)
{
    ssa_operand_t lhs = compile_rvalue(ssa, cond->lhs);
    ssa_operand_t rhs = compile_rvalue(ssa, cond->rhs);

    ssa_step_t step = {
        .opcode = eOpCompare,
        .type = ssa_get_bool_type(ssa),
        .compare = {
            .op = cond->compare,
            .lhs = lhs,
            .rhs = rhs
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t compile_compare_bool(ssa_t *ssa, const hlir_t *cond)
{
    ssa_operand_t op = {
        .kind = eOperandImm,
        .value = value_bool_new(ssa, cond)
    };

    return op;
}

static ssa_operand_t compile_compare(ssa_t *ssa, const hlir_t *cond)
{
    hlir_kind_t kind = get_hlir_kind(cond);
    switch (kind)
    {
    case eHlirCompare:
        return compile_compare_compare(ssa, cond);

    case eHlirBoolLiteral:
        return compile_compare_bool(ssa, cond);

    default:
        report(ssa->reports, eInternal, get_hlir_node(cond), "compile-compare %s", hlir_kind_to_string(kind));
        return operand_empty();
    }
}

static ssa_operand_t compile_loop(ssa_t *ssa, const hlir_t *stmt)
{
    ssa_block_t *loop = block_gen(ssa, "loop");
    ssa_block_t *tail = block_gen(ssa, "tail");

    ssa_step_t step = {
        .opcode = eOpJmp,
        .type = type_empty_new("empty"),
        .jmp = {
            .label = operand_bb(loop)
        }
    };

    add_step(ssa, step);

    ssa->currentBlock = loop;
    compile_stmt(ssa, stmt->then);

    ssa_step_t ret = {
        .opcode = eOpBranch,
        .type = type_empty_new("empty"),
        .branch = {
            .cond = compile_compare(ssa, stmt->cond),
            .truthy = operand_bb(loop),
            .falsey = operand_bb(tail)
        }
    };

    add_step(ssa, ret);

    ssa->currentBlock = tail;

    return operand_bb(tail);
}

static ssa_operand_t compile_branch(ssa_t *ssa, const hlir_t *stmt)
{
    ssa_block_t *then = block_gen(ssa, "then");
    ssa_block_t *other = stmt->other != NULL ? block_gen(ssa, "other") : NULL;
    ssa_block_t *tail = block_gen(ssa, "tail");

    ssa_step_t ret = {
        .opcode = eOpJmp,
        .type = type_empty_new("empty"),
        .jmp = {
            .label = operand_bb(tail)
        }
    };

    ssa_step_t step = {
        .opcode = eOpBranch,
        .type = type_empty_new("empty"),
        .branch = {
            .cond = compile_compare(ssa, stmt->cond),
            .truthy = operand_bb(then),
            .falsey = other != NULL ? operand_bb(other) : operand_bb(tail)
        }
    };

    add_step(ssa, step);

    // if true
    ssa->currentBlock = then;
    compile_stmt(ssa, stmt->then);
    add_step(ssa, ret);

    // if false
    if (other != NULL) 
    {
        ssa->currentBlock = other;
        compile_stmt(ssa, stmt->other);
        add_step(ssa, ret);
    }

    ssa->currentBlock = tail;

    return operand_bb(tail);
}

static ssa_operand_t compile_return(ssa_t *ssa, const hlir_t *stmt)
{
    const hlir_t *result = stmt->result;
    ssa_operand_t op = result != NULL ? compile_rvalue(ssa, result) : operand_empty();

    ssa_step_t step = {
        .opcode = eOpReturn,
        .type = type_empty_new("empty"),
        .ret = {
            .value = op,
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t compile_stmt(ssa_t *ssa, const hlir_t *stmt)
{
    hlir_kind_t kind = get_hlir_kind(stmt);
    switch (kind)
    {
    case eHlirStmts:
        return compile_stmts(ssa, stmt);

    case eHlirCall:
        return compile_call(ssa, stmt);

    case eHlirAssign:
        return compile_assign(ssa, stmt);

    case eHlirLoop:
        return compile_loop(ssa, stmt);

    case eHlirBranch:
        return compile_branch(ssa, stmt);

    case eHlirLoad:
        return compile_rvalue(ssa, stmt);

    case eHlirReturn:
        return compile_return(ssa, stmt);

    default:
        report(ssa->reports, eInternal, get_hlir_node(stmt), "compile-stmt %s", hlir_kind_to_string(kind));
        return operand_empty();
    }
}

static void compile_flow(ssa_t *ssa, ssa_flow_t *flow)
{
    ssa_block_t *block = block_new("entry");
    flow->entry = block;
    ssa->currentBlock = block;
    ssa->stepIdx = 0;
    ssa->blockIdx = 0;
}

static void compile_global(ssa_t *ssa, const hlir_t *global)
{
    ssa_flow_t *flow = map_get_ptr(ssa->globals, global);

    if (is_imported(flow->linkage))
    {
        return;
    }

    compile_flow(ssa, flow);

    ssa_operand_t result = global->value == NULL ? operand_empty() : compile_rvalue(ssa, global->value);
    ssa_step_t step = {
        .opcode = eOpReturn,
        .type = type_empty_new("empty"),
        .ret = {
            .value = result
        }
    };
    add_step(ssa, step);
}

static void compile_function(ssa_t *ssa, const hlir_t *function)
{
    ssa_flow_t *flow = map_get_ptr(ssa->functions, function);
    CTASSERT(flow != NULL);

    if (is_imported(flow->linkage))
    {
        return;
    }

    size_t totalLocals = vector_len(function->locals);
    size_t totalParams = vector_len(function->params);

    ssa->currentLocals = map_optimal(totalLocals);
    ssa->currentParams = map_optimal(totalParams);

    flow->locals = vector_of(totalLocals);
    
    for (size_t i = 0; i < totalLocals; i++) 
    {
        const hlir_t *local = vector_get(function->locals, i);
        map_set_ptr(ssa->currentLocals, local, (void*)(uintptr_t)i);
        vector_set(flow->locals, i, type_new(ssa, get_hlir_type(local)));
    }

    for (size_t i = 0; i < totalParams; i++)
    {
        const hlir_t *param = vector_get(function->params, i);
        map_set_ptr(ssa->currentParams, param, (void*)(uintptr_t)i);
    }

    compile_flow(ssa, flow);

    // TODO: idk if this is right
    if (function->body == NULL) 
    { 
        map_set(ssa->importedSymbols, function->name, flow);
        return;
    }

    compile_stmt(ssa, function->body);
}

ssa_module_t *ssa_gen_module(reports_t *reports, vector_t *mods)
{
    ssa_t ssa = { 
        .reports = reports,
        .aggregates = map_optimal(0x1000),
        .globals = map_optimal(0x1000),
        .functions = map_optimal(0x1000),
        .strings = set_new(0x1000),
        .importedSymbols = map_optimal(0x1000)
    };

    size_t len = vector_len(mods);
    for (size_t i = 0; i < len; i++)
    {
        hlir_t *mod = vector_get(mods, i);
        for (size_t j = 0; j < vector_len(mod->globals); j++)
        {
            fwd_global(&ssa, vector_get(mod->globals, j));
        }

        for (size_t j = 0; j < vector_len(mod->functions); j++)
        {
            fwd_function(&ssa, vector_get(mod->functions, j));
        }
    }

    for (size_t i = 0; i < len; i++)
    {
        hlir_t *mod = vector_get(mods, i);
        for (size_t j = 0; j < vector_len(mod->globals); j++)
        {
            compile_global(&ssa, vector_get(mod->globals, j));
        }

        for (size_t j = 0; j < vector_len(mod->functions); j++)
        {
            compile_function(&ssa, vector_get(mod->functions, j));
        }
    }

    section_t symbols = {
        .types = map_values(ssa.aggregates),
        .globals = map_values(ssa.globals),
        .functions = map_values(ssa.functions)
    };

    ssa_module_t *mod = ctu_malloc(sizeof(ssa_module_t));
    mod->symbols = symbols;

    return mod;
}
