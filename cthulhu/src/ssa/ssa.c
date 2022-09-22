#include "cthulhu/ssa/ssa.h"

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/query.h"

#include "base/macros.h"
#include "base/memory.h"
#include "base/util.h"

#include "std/map.h"
#include "std/vector.h"
#include "std/str.h"

#include "report/report.h"

typedef struct {
    reports_t *reports;
    
    map_t *globals;
    map_t *functions;

    map_t *importedSymbols;

    block_t *currentBlock;
    size_t stepIdx;
    size_t blockIdx;
} ssa_t;

static block_t *block_new(const char *id) 
{
    block_t *block = ctu_malloc(sizeof(block_t));
    block->steps = vector_new(16);
    block->id = id;
    return block;
}

static typekind_t get_type_kind(ssa_t *ssa, const hlir_t *hlir)
{
    hlir_kind_t kind = get_hlir_kind(hlir);
    switch (kind)
    {
    case eHlirDigit: return eTypeDigit;
    case eHlirBool: return eTypeBool;
    case eHlirPointer: // TODO: frontend should also have opaque pointer type, this is a hack
        return hlir_is(hlir->ptr, eHlirUnit) ? eTypeOpaque : eTypePointer;
    case eHlirUnit: return eTypeUnit;
    case eHlirEmpty: return eTypeEmpty;

    default:
        report(ssa->reports, eInternal, get_hlir_node(hlir), "no respective ssa type for %s", hlir_kind_to_string(kind));
        return eTypeEmpty;
    }
}

static type_t *type_new(ssa_t *ssa, const hlir_t *type)
{
    const hlir_t *real = hlir_follow_type(type);
    type_t *it = ctu_malloc(sizeof(type_t));
    it->name = get_hlir_name(real);
    it->kind = get_type_kind(ssa, real);

    if (hlir_is(real, eHlirPointer))
    {
        it->ptr = type_new(ssa, real->ptr);
    }
    
    return it;
}

static operand_t add_step(ssa_t *ssa, step_t step)
{
    step_t *ptr = BOX(step);
    if (ptr->id == NULL)
    {
        ptr->id = format("%zu", ssa->stepIdx++);
    }
    vector_push(&ssa->currentBlock->steps, ptr);
    operand_t op = {
        .kind = eOperandReg,
        .type = step.type,
        .reg = ptr
    };
    return op;
}

static flow_t *flow_new(const hlir_t *symbol)
{
    flow_t *flow = ctu_malloc(sizeof(flow_t));
    flow->name = get_hlir_name(symbol);
    flow->entry = NULL;
    return flow;
}

static void fwd_global(ssa_t *ssa, const hlir_t *global)
{
    flow_t *flow = flow_new(global);
    map_set_ptr(ssa->globals, global, flow);
}

static void fwd_function(ssa_t *ssa, const hlir_t *function)
{
    flow_t *flow = flow_new(function);
    map_set_ptr(ssa->functions, function, flow);
}

static operand_t compile_rvalue(ssa_t *ssa, const hlir_t *hlir);
static operand_t compile_stmt(ssa_t *ssa, const hlir_t *stmt);

static operand_t compile_digit(ssa_t *ssa, const hlir_t *hlir)
{
    imm_t imm;
    mpz_init_set(imm.digit, hlir->digit);

    operand_t op = {
        .kind = eOperandImm,
        .type = type_new(ssa, get_hlir_type(hlir)),
        .imm = imm
    };

    return op;
}

static operand_t compile_binary(ssa_t *ssa, const hlir_t *hlir)
{
    operand_t lhs = compile_rvalue(ssa, hlir->lhs);
    operand_t rhs = compile_rvalue(ssa, hlir->rhs);
    
    step_t step = {
        .opcode = eOpBinary,
        .type = type_new(ssa, get_hlir_type(hlir)),
        .binary = hlir->binary,
        .lhs = lhs,
        .rhs = rhs
    };

    return add_step(ssa, step);
}

static operand_t compile_load(ssa_t *ssa, const hlir_t *hlir)
{
    operand_t name = compile_rvalue(ssa, hlir->read);
    step_t step = {
        .opcode = eOpLoad,
        .type = type_new(ssa, get_hlir_type(hlir)),
        .value = name
    };

    return add_step(ssa, step);
}

static operand_t compile_global_ref(ssa_t *ssa, const hlir_t *hlir)
{
    const flow_t *ref = map_get_ptr(ssa->globals, hlir);

    operand_t op = {
        .kind = eOperandGlobal,
        .type = type_new(ssa, get_hlir_type(hlir)),
        .flow = ref
    };

    return op;
}

static operand_t compile_cast(ssa_t *ssa, const hlir_t* hlir)
{
    operand_t op = compile_rvalue(ssa, hlir->expr);

    step_t step = {
        .opcode = eOpCast,
        .type = type_new(ssa, get_hlir_type(hlir)),
        .cast = hlir->cast,
        .operand = op
    };

    return add_step(ssa, step);
}

static operand_t compile_rvalue(ssa_t *ssa, const hlir_t *hlir)
{
    hlir_kind_t kind = get_hlir_kind(hlir);
    switch (kind)
    {
    case eHlirDigitLiteral:
        return compile_digit(ssa, hlir);

    case eHlirBinary:
        return compile_binary(ssa, hlir);

    case eHlirLoad:
        return compile_load(ssa, hlir);

    case eHlirGlobal:
        return compile_global_ref(ssa, hlir);

    case eHlirCast:
        return compile_cast(ssa, hlir);

    default:
        report(ssa->reports, eInternal, get_hlir_node(hlir), "compile-rvalue %s", hlir_kind_to_string(kind));
        return (operand_t){.kind = eOperandEmpty};
    }
}

static operand_t operand_bb(const block_t *dst)
{
    operand_t op = {
        .kind = eOperandBlock,
        .bb = dst
    };

    return op;
}

static operand_t compile_stmts(ssa_t *ssa, const hlir_t *stmt)
{
    block_t *oldBlock = ssa->currentBlock;
    ssa->currentBlock = block_new(format("%zu", ssa->blockIdx++));
    
    for (size_t i = 0; i < vector_len(stmt->stmts); i++)
    {
        compile_stmt(ssa, vector_get(stmt->stmts, i));
    }

    ssa->currentBlock = oldBlock;
    return operand_bb(ssa->currentBlock);
}

static operand_t compile_stmt(ssa_t *ssa, const hlir_t *stmt)
{
    hlir_kind_t kind = get_hlir_kind(stmt);
    switch (kind)
    {
    case eHlirStmts:
        return compile_stmts(ssa, stmt);
    default:
        report(ssa->reports, eInternal, get_hlir_node(stmt), "compile-stmt %s", hlir_kind_to_string(kind));
        return (operand_t){.kind = eOperandEmpty};
    }
}

static void compile_flow(ssa_t *ssa, flow_t *flow)
{
    block_t *block = block_new("entry");
    flow->entry = block;
    ssa->currentBlock = block;
    ssa->stepIdx = 0;
    ssa->blockIdx = 0;
}

static void compile_global(ssa_t *ssa, const hlir_t *global)
{
    flow_t *flow = map_get_ptr(ssa->globals, global);
    compile_flow(ssa, flow);

    operand_t result = compile_rvalue(ssa, global->value);
    step_t step = {
        .opcode = eOpReturn,
        .type = type_new(ssa, get_hlir_type(global)),
        .value = result
    };
    add_step(ssa, step);
}

static void compile_function(ssa_t *ssa, const hlir_t *function)
{
    flow_t *flow = map_get_ptr(ssa->functions, function);
    compile_flow(ssa, flow);

    compile_stmt(ssa, function->body);
}

module_t *emit_module(reports_t *reports, vector_t *mods)
{
    ssa_t ssa = { 
        .reports = reports,
        .globals = map_optimal(0x1000),
        .functions = map_optimal(0x1000),
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

    module_t *mod = ctu_malloc(sizeof(module_t));
    mod->symbols = symbols;

    return mod;
}
