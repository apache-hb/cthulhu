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
        .reg = ptr
    };
    return op;
}

static void fwd_global(ssa_t *ssa, const hlir_t *global)
{
    flow_t *flow = ctu_malloc(sizeof(flow_t));
    flow->name = get_hlir_name(global);
    flow->entry = NULL;
    map_set_ptr(ssa->globals, global, flow);
}

static operand_t compile_rvalue(ssa_t *ssa, const hlir_t *hlir);

static operand_t compile_digit(const hlir_t *hlir)
{
    imm_t imm;
    mpz_init_set(imm.digit, hlir->digit);

    operand_t op = {
        .kind = eOperandImm,
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
        .value = name
    };

    return add_step(ssa, step);
}

static operand_t compile_global_ref(ssa_t *ssa, const hlir_t *hlir)
{
    const flow_t *ref = map_get_ptr(ssa->globals, hlir);

    operand_t op = {
        .kind = eOperandGlobal,
        .flow = ref
    };

    return op;
}

static operand_t compile_rvalue(ssa_t *ssa, const hlir_t *hlir)
{
    hlir_kind_t kind = get_hlir_kind(hlir);
    switch (kind)
    {
    case eHlirDigitLiteral:
        return compile_digit(hlir);

    case eHlirBinary:
        return compile_binary(ssa, hlir);

    case eHlirLoad:
        return compile_load(ssa, hlir);

    case eHlirGlobal:
        return compile_global_ref(ssa, hlir);

    default:
        report(ssa->reports, eInternal, get_hlir_node(hlir), "compile-rvalue %s", hlir_kind_to_string(kind));
        return (operand_t){.kind = eOperandEmpty};
    }
}

static void compile_global(ssa_t *ssa, const hlir_t *global)
{
    flow_t *flow = map_get_ptr(ssa->globals, global);
    block_t *block = block_new("entry");
    flow->entry = block;
    ssa->currentBlock = block;
    ssa->stepIdx = 0;
    ssa->blockIdx = 0;

    operand_t result = compile_rvalue(ssa, global->value);
    step_t step = {
        .opcode = eOpReturn,
        .value = result
    };
    add_step(ssa, step);
}

module_t *emit_module(reports_t *reports, vector_t *mods)
{
    ssa_t ssa = { 
        .reports = reports,
        .globals = map_optimal(0x1000)
    };

    size_t len = vector_len(mods);
    for (size_t i = 0; i < len; i++)
    {
        hlir_t *mod = vector_get(mods, i);
        for (size_t j = 0; j < vector_len(mod->globals); j++)
        {
            fwd_global(&ssa, vector_get(mod->globals, j));
        }
    }

    for (size_t i = 0; i < len; i++)
    {
        hlir_t *mod = vector_get(mods, i);
        for (size_t j = 0; j < vector_len(mod->globals); j++)
        {
            compile_global(&ssa, vector_get(mod->globals, j));
        }
    }

    module_t *mod = ctu_malloc(sizeof(module_t));
    mod->flows = map_values(ssa.globals);

    return mod;
}
