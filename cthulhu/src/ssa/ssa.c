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

typedef struct {
    reports_t *reports;
    
    map_t *globals;
    map_t *functions;

    set_t *strings;

    map_t *importedSymbols; // map_t<const char *, const hlir_t *>

    map_t *currentLocals; // map_t<const hlir_t *, size_t> // map of local variables to their index in the locals vector

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
    case eHlirBool: return eTypeBool;
    case eHlirString: return eTypeString;
    case eHlirPointer: // TODO: frontend should also have opaque pointer type, this is a hack
        return eTypePointer; // hlir_is(hlir->ptr, eHlirUnit) ? eTypeOpaque : eTypePointer;
    case eHlirUnit: return eTypeUnit;
    case eHlirEmpty: return eTypeEmpty;
    case eHlirFunction: return eTypeSignature;

    default:
        report(ssa->reports, eInternal, get_hlir_node(hlir), "no respective ssa type for %s", hlir_kind_to_string(kind));
        return eTypeEmpty;
    }
}

static ssa_type_t *ssa_type_new(ssa_t *ssa, ssa_kind_t kind)
{
    UNUSED(ssa);
    ssa_type_t *it = ctu_malloc(sizeof(ssa_type_t));
    it->kind = kind;
    it->name = NULL;
    return it;
}

static ssa_type_t *type_new(ssa_t *ssa, const hlir_t *type)
{
    const hlir_t *real = hlir_follow_type(type);
    hlir_kind_t kind = get_hlir_kind(real);
    ssa_type_t *it = ssa_type_new(ssa, get_type_kind(ssa, real));
    it->name = get_hlir_name(real);

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
        for (size_t i = 0; i < vector_len(real->params); i++) {
            ssa_type_t *arg = type_new(ssa, vector_get(real->params, i));
            vector_set(it->args, i, arg);
        }
        break;
    }

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

static ssa_flow_t *flow_new(ssa_t *ssa, const hlir_t *symbol, ssa_type_t *type)
{
    UNUSED(ssa);

    ssa_flow_t *flow = ctu_malloc(sizeof(ssa_flow_t));
    flow->name = get_hlir_name(symbol);
    flow->type = type;
    flow->entry = NULL;

    // TODO: a little iffy
    flow->linkage = symbol->attributes->linkage;
    flow->visibility = symbol->attributes->visibility;

    return flow;
}

static ssa_flow_t *global_new(ssa_t *ssa, const hlir_t *global)
{
    ssa_type_t *type = type_new(ssa, get_hlir_type(global));
    ssa_flow_t *flow = flow_new(ssa, global, type);
    flow->value = NULL;
    return flow;
}

static ssa_flow_t *function_new(ssa_t *ssa, const hlir_t *function)
{
    ssa_type_t *type = type_new(ssa, get_hlir_type(function));
    ssa_flow_t *flow = flow_new(ssa, function, type);
    flow->locals = vector_new(0);
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

static ssa_type_t *ssa_get_digit_type(ssa_t *ssa, const hlir_t *digit)
{
    CTASSERTF(hlir_is(digit, eHlirDigitLiteral), "expected digit, got %s", hlir_kind_to_string(get_hlir_kind(digit)));

    const hlir_t *type = hlir_follow_type(get_hlir_type(digit));

    CTASSERT(hlir_is(type, eHlirDigit));

    ssa_type_t *it = ssa_type_new(ssa, eTypeDigit);
    it->name = get_hlir_name(type);
    it->digit = type->width;
    it->sign = type->sign;

    return it;
}

static ssa_type_t *ssa_get_bool_type(ssa_t *ssa)
{
    ssa_type_t *it = ssa_type_new(ssa, eTypeBool);
    it->name = "bool"; // TODO: match frontend name

    return it;
}

static ssa_type_t *ssa_get_string_type(ssa_t *ssa)
{
    ssa_type_t *it = ssa_type_new(ssa, eTypeString);
    it->name = "string"; // TODO: match frontend name

    return it;
}

static ssa_value_t *ssa_value_digit_new(ssa_t *ssa, const hlir_t *hlir)
{
    ssa_value_t *it = ssa_value_new(ssa_get_digit_type(ssa, hlir), true);

    mpz_init_set(it->digit, hlir->digit);

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

static ssa_operand_t compile_string(ssa_t *ssa, const hlir_t *hlir)
{
    // TODO: string
    ssa_operand_t op = {
        .kind = eOperandImm,
        .value = value_string_new(ssa, hlir)
    };

    return op;
}

static ssa_operand_t compile_binary(ssa_t *ssa, const hlir_t *hlir)
{
    ssa_operand_t lhs = compile_rvalue(ssa, hlir->lhs);
    ssa_operand_t rhs = compile_rvalue(ssa, hlir->rhs);

    ssa_step_t step = {
        .opcode = eOpBinary,
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

    ssa_step_t step = {
        .opcode = eOpCast,
        .cast = {
            .op = hlir->cast,
            .operand = op,

            .type = type_new(ssa, get_hlir_type(hlir))
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
        args[i] = compile_rvalue(ssa, vector_get(hlir->args, i));
    }

    ssa_operand_t func = compile_rvalue(ssa, hlir->call);

    ssa_step_t step = {
        .opcode = eOpCall,
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

static ssa_operand_t compile_lvalue(ssa_t *ssa, const hlir_t *hlir)
{
    hlir_kind_t kind = get_hlir_kind(hlir);
    switch (kind)
    {
    case eHlirGlobal:
        return compile_global_lvalue(ssa, hlir);

    case eHlirLocal:
        return compile_local_lvalue(ssa, hlir);

    default:
        report(ssa->reports, eInternal, get_hlir_node(hlir), "compile-lvalue %s", hlir_kind_to_string(kind));
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

    case eHlirStringLiteral:
        return compile_string(ssa, hlir);

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
        .jmp = {
            .label = operand_bb(loop)
        }
    };

    add_step(ssa, step);

    ssa->currentBlock = loop;
    compile_stmt(ssa, stmt->then);

    ssa_step_t ret = {
        .opcode = eOpBranch,
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
        .jmp = {
            .label = operand_bb(tail)
        }
    };

    ssa_step_t step = {
        .opcode = eOpBranch,
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
    compile_flow(ssa, flow);

    ssa_operand_t result = global->value == NULL ? operand_empty() : compile_rvalue(ssa, global->value);
    ssa_step_t step = {
        .opcode = eOpReturn,
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

    size_t len = vector_len(function->locals);
    ssa->currentLocals = map_optimal(len);
    flow->locals = vector_of(len);
    
    for (size_t i = 0; i < len; i++) 
    {
        const hlir_t *local = vector_get(function->locals, i);
        map_set_ptr(ssa->currentLocals, local, (void*)(uintptr_t)i);
        vector_set(flow->locals, i, type_new(ssa, get_hlir_type(local)));
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
        .globals = map_values(ssa.globals),
        .functions = map_values(ssa.functions)
    };

    ssa_module_t *mod = ctu_malloc(sizeof(ssa_module_t));
    mod->symbols = symbols;

    return mod;
}
