#include "cthulhu/hlir/ops.h"
#include "cthulhu/ssa/ssa.h"

#include "report/report.h"
#include "std/vector.h"
#include "std/str.h"
#include "std/set.h"

#include "base/macros.h"
#include "base/panic.h"

#include "common.h"

#include <stdio.h>

typedef struct {
    reports_t *reports;
    set_t *blocks;
} emit_t;

static const char *emit_type(emit_t *emit, const ssa_type_t *type)
{
    UNUSED(emit);

    if (type == NULL)
    {
        return "nil";
    }

    switch (type->kind)
    {
    case eTypeBool: return "bool";
    case eTypeDigit: return "digit";
    case eTypeEmpty: return "empty";
    // case eTypeOpaque: return "ptr";
    // case eTypePointer: return format("%s*", type->ptr);
    case eTypeStruct: return format("struct(%s)", type->name);
    case eTypeUnit: return "unit";
    default: return format("error(%s)", type->name);
    }
}

static const char *emit_value(emit_t *emit, const ssa_value_t *value)
{
    UNUSED(emit);
    CTASSERT(value != NULL);

    ssa_kind_t kind = ssa_get_value_kind(value);

    switch (kind) 
    {
    case eTypeDigit: 
        return format("%s", mpz_get_str(NULL, 10, value->digit));
    case eTypeBool:
        return format("%s", value->boolean ? "true" : "false");
    case eTypeString: {
        string_view_t string = value->string;
        return format("\"%s\"", str_normalizen(string.data, string.size));
    }

    default: return format("error(%s)", ssa_kind_name(kind));
    }
}

static const char *get_flow_name(const ssa_flow_t *flow)
{
    CTASSERT(flow != NULL);

    return flow->name ? flow->name : "unknown";
}

static const char *emit_operand(emit_t *emit, set_t *edges, ssa_operand_t op)
{
    switch (op.kind)
    {
    case eOperandEmpty: 
        return "";

    case eOperandReg: 
        return format("%%%s", op.vreg->id);

    case eOperandBlock: 
        set_add_ptr(edges, op.bb);
        return format(".%s", op.bb->id);

    case eOperandImm:
        return format("$%s", emit_value(emit, op.value));

    case eOperandGlobal: 
        return format("&%s", get_flow_name(op.global));
        
    case eOperandLocal: 
        return format("local[%zu]", op.local);

    case eOperandFunction: 
        return format("%s", get_flow_name(op.function));

    default: 
        report(emit->reports, eInternal, NULL, "unhandled operand kind %d", (int)op.kind);
        return format("err(%d)", ssa_operand_name(op.kind));
    }
}

static const char *emit_operand_list(emit_t *emit, set_t *edges, ssa_operand_t *ops, size_t count)
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

static void add_edge(set_t *edges, ssa_operand_t op)
{
    if (op.kind == eOperandBlock) 
    {
        set_add_ptr(edges, op.bb);
    }
}

static void emit_step(emit_t *emit, set_t *edges, ssa_step_t *step)
{
    switch (step->opcode)
    {
    case eOpImm: {
        ssa_imm_t imm = step->imm;
        printf("  %%%s = %s\n", step->id, emit_operand(emit, edges, imm.value));
        break;
    }

    case eOpBinary: {
        ssa_binary_t binary = step->binary;
        printf("  %%%s = binary %s %s %s\n", step->id, binary_name(binary.op), emit_operand(emit, edges, binary.lhs), emit_operand(emit, edges, binary.rhs));
        break;
    }

    case eOpUnary: {
        ssa_unary_t unary = step->unary;
        printf("  %%%s = unary %s %s\n", step->id, unary_name(unary.op), emit_operand(emit, edges, unary.operand));
        break;
    }

    case eOpLoad: {
        ssa_load_t load = step->load;
        printf("  %%%s = load %s\n", step->id, emit_operand(emit, edges, load.src));
        break;
    }

    case eOpReturn: {
        ssa_return_t ret = step->ret;
        printf("  ret %s\n", emit_operand(emit, edges, ret.value));
        break;
    }

    case eOpCast: {
        ssa_cast_t cast = step->cast;
        printf("  %%%s = %s<%s> %s\n", step->id, cast_name(cast.op), emit_type(emit, cast.type), emit_operand(emit, edges, cast.operand));
        break;
    }

    case eOpCall: {
        ssa_call_t call = step->call;
        printf("  %%%s = call %s(%s)\n", step->id, emit_operand(emit, edges, call.symbol), emit_operand_list(emit, edges, call.args, call.len));
        break;
    }

    case eOpJmp: {
        ssa_jmp_t jmp = step->jmp;
        printf("  jmp %s\n", emit_operand(emit, edges, jmp.label));
        add_edge(edges, jmp.label);
        break;
    }

    case eOpBranch: {
        ssa_branch_t branch = step->branch;
        printf("  br %s %s %s\n", emit_operand(emit, edges, branch.cond), emit_operand(emit, edges, branch.truthy), emit_operand(emit, edges, branch.falsey));
        add_edge(edges, branch.truthy);
        add_edge(edges, branch.falsey);
        break;
    }

    case eOpStore: {
        ssa_store_t store = step->store;
        printf("  store %s = %s\n", emit_operand(emit, edges, store.dst), emit_operand(emit, edges, store.src));
        break;
    }

    case eOpCompare: {
        ssa_compare_t compare = step->compare;
        printf("  %%%s = cmp %s %s %s\n", step->id, compare_name(compare.op), emit_operand(emit, edges, compare.lhs), emit_operand(emit, edges, compare.rhs));
        break;
    }

    default:
        printf("  <error> %s\n", ssa_opcode_name(step->opcode));
        break;
    }
}

static void emit_block(emit_t *emit, const ssa_block_t *block)
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
        ssa_step_t *step = vector_get(block->steps, i);
        emit_step(emit, edges, step);
    }

    set_iter_t iter = set_iter(edges);
    while (set_has_next(&iter))
    {
        const ssa_block_t *edge = set_next(&iter);
        emit_block(emit, edge);
    }
}

static void emit_flow(emit_t *emit, const ssa_flow_t *flow)
{
    if (flow->value != NULL) 
    {
        printf("%s = %s\n", flow->name, emit_value(emit, flow->value));
    }
    else
    {
        printf("%s:\n", flow->name);
        emit_block(emit, flow->entry);
    }
}

static void emit_flows(emit_t *emit, vector_t *vec)
{
    for (size_t i = 0; i < vector_len(vec); i++)
    {
        emit_flow(emit, vector_get(vec, i));
    }
}

static void emit_globals(emit_t *emit, vector_t *globals)
{
    emit_flows(emit, globals);
}

static void emit_functions(emit_t *emit, vector_t *functions)
{
    emit_flows(emit, functions);
}

void ssa_emit_module(reports_t *reports, ssa_module_t *mod)
{
    emit_t emit = {
        .reports = reports,
        .blocks = set_new(0x100)
    };

    section_t symbols = mod->symbols;

    emit_globals(&emit, symbols.globals);
    emit_functions(&emit, symbols.functions);
}
