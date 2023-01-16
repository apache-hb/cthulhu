#include "cthulhu/hlir/ops.h"
#include "cthulhu/ssa/ssa.h"

#include "std/vector.h"
#include "std/str.h"
#include "std/set.h"

#include "base/macros.h"

#include <stdio.h>

typedef struct {
    reports_t *reports;
    set_t *blocks;
} emit_t;

static const char *emit_type(emit_t *emit, const type_t *type)
{
    if (type == NULL)
    {
        return "nil";
    }

    switch (type->kind)
    {
    case eTypeBool: return "bool";
    case eTypeDigit: return "digit";
    case eTypeEmpty: return "empty";
    case eTypeOpaque: return "ptr";
    case eTypePointer: return format("%s*", type->ptr);
    case eTypeStruct: return format(":%s", type->name);
    case eTypeUnit: return "unit";
    default: return format("err(%s)", type->name);
    }
}

static const char *emit_operand(emit_t *emit, set_t *edges, operand_t op)
{
    UNUSED(emit);

    switch (op.kind)
    {
    case eOperandEmpty: return "";
    case eOperandReg: return format("%%%s", op.reg->id);
    case eOperandBlock: 
        set_add_ptr(edges, op.bb);
        return format(".%s", op.bb->id);
    case eOperandImm: return format("$%s", mpz_get_str(NULL, 10, op.imm.digit));
    case eOperandGlobal: return format("&%s", op.flow->name);
    case eOperandLocal: return format("local[%zu]", op.local);
    case eOperandFunction: return format("%s", op.flow->name);
    default: return "unknown";
    }
}

static const char *emit_operand_list(emit_t *emit, set_t *edges, operand_t *ops, size_t count)
{
    if (count == 0)
    {
        return "";
    }

    vector_t *result = vector_of(count);
    for (size_t i = 0; i < count; ++i)
    {
        vector_set(result, i, (void*)emit_operand(emit, edges, ops[i]));
    }

    return str_join(", ", result);
}

static void add_edge(set_t *edges, operand_t op)
{
    if (op.kind == eOperandBlock) 
    {
        set_add_ptr(edges, op.bb);
    }
}

static void emit_step(emit_t *emit, set_t *edges, step_t *step)
{
    switch (step->opcode)
    {
    case eOpValue:
        printf("  %%%s = %s\n", step->id, emit_operand(emit, edges, step->value));
        break;

    case eOpBinary:
        printf("  %%%s = binary %s %s %s\n", step->id, binary_name(step->binary), emit_operand(emit, edges, step->lhs), emit_operand(emit, edges, step->rhs));
        break;

    case eOpLoad:
        printf("  %%%s = load %s\n", step->id, emit_operand(emit, edges, step->value));
        break;

    case eOpReturn:
        printf("  ret %s %s\n", emit_type(emit, step->type), emit_operand(emit, edges, step->value));
        break;

    case eOpCast:
        printf("  %%%s = %s<%s> %s\n", step->id, cast_name(step->cast), emit_type(emit, step->type), emit_operand(emit, edges, step->operand));
        break;

    case eOpCall: 
        printf("  %%%s = call %s(%s)\n", step->id, emit_operand(emit, edges, step->symbol), emit_operand_list(emit, edges, step->args, step->len));
        break;

    case eOpJmp:
        printf("  jmp %s\n", emit_operand(emit, edges, step->label));
        add_edge(edges, step->label);
        break;

    case eOpBranch:
        printf("  br %s %s else %s\n", emit_operand(emit, edges, step->cond), emit_operand(emit, edges, step->label), emit_operand(emit, edges, step->other));
        add_edge(edges, step->label);
        add_edge(edges, step->other);
        break;

    case eOpStore:
        printf("  store %s %s\n", emit_operand(emit, edges, step->dst), emit_operand(emit, edges, step->src));
        break;

    case eOpCompare:
        printf("  %%%s = cmp %s %s %s\n", step->id, compare_name(step->compare), emit_operand(emit, edges, step->lhs), emit_operand(emit, edges, step->rhs));
        break;

    default:
        printf("  <error> %d\n", (int)step->opcode);
        break;
    }
}

static void emit_block(emit_t *emit, const block_t *block)
{
    if (set_contains_ptr(emit->blocks, block)) 
    {
        return;
    }
    set_add_ptr(emit->blocks, block);

    size_t len = vector_len(block->steps);
    set_t *edges = set_new(len);

    printf(".%s:\n", block->id);
    for (size_t i = 0; i < len; i++) 
    {
        step_t *step = vector_get(block->steps, i);
        emit_step(emit, edges, step);
    }

    set_iter_t iter = set_iter(edges);
    while (set_has_next(&iter))
    {
        const block_t *edge = set_next(&iter);
        emit_block(emit, edge);
    }
}

static void emit_flow(emit_t *emit, const flow_t *flow)
{
    printf("%s:\n", flow->name);
    emit_block(emit, flow->entry);
}

void emit_flows(emit_t *emit, vector_t *vec)
{
    for (size_t i = 0; i < vector_len(vec); i++)
    {
        emit_flow(emit, vector_get(vec, i));
    }
}

void eval_module(reports_t *reports, module_t *mod)
{
    emit_t emit = {
        .reports = reports,
        .blocks = set_new(0x100)
    };

    section_t symbols = mod->symbols;

    emit_flows(&emit, symbols.globals);
    emit_flows(&emit, symbols.functions);
}
