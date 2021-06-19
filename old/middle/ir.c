#include "ir.h"
#include "bison.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "cthulhu/util/report.h"

typedef struct {
    const char *name;
    operand_t op;
} name_op_t;

typedef struct {
    flow_t *flow;
    name_op_t *vars;
    size_t len, size;
} flow_gen_t;

static unit_t unit_new(const char *name, size_t size) {
    unit_t unit = { name, malloc(sizeof(flow_t) * size), size, 0 };
    return unit;
}

static void unit_add(unit_t *unit, flow_t flow) {
    ENSURE_SIZE(unit->flows, unit->len, unit->size, sizeof(flow_t), 4);
    unit->flows[unit->len++] = flow;
}

static flow_gen_t flowgen_new(flow_t *flow) {
    flow_gen_t gen = { flow, malloc(sizeof(name_op_t) * 4), 0, 4 };
    return gen;
}

static operand_t create_operand(opkind_t kind) {
    operand_t op = { kind, { .vreg = SIZE_MAX }, SIZE_MAX };
    return op;
}

static operand_t create_name(const char *text) {
    ASSERT(text != NULL);

    operand_t op = create_operand(NAME);
    op.name = text;
    return op;
}

static operand_t get_var(flow_gen_t *ctx, const char *name) {
    for (size_t i = 0; i < ctx->len; i++) {
        name_op_t *pair = ctx->vars + i;
        if (strcmp(pair->name, name) == 0)
            return pair->op;
    }

    reportf("oh no");
    return create_operand(NONE);
}


static void add_var(flow_gen_t *ctx, const char *name, operand_t op) {
    ENSURE_SIZE(ctx->vars, ctx->len, ctx->size, sizeof(name_op_t), 4);
    name_op_t pair = { name, op };
    ctx->vars[ctx->len++] = pair;
}

static flow_t flow_new(const char *name, opkind_t result, size_t size) {
    flow_t flow = { name, result, malloc(sizeof(op_t) * size), size, 0 };
    return flow;
}

static size_t flow_add(flow_t *flow, op_t op) {
    ENSURE_SIZE(flow->ops, flow->len, flow->size, sizeof(op_t), 32);
    flow->ops[flow->len] = op;
    return flow->len++;
}

static size_t flow_gen_add(flow_gen_t *gen, op_t op) {
    return flow_add(gen->flow, op);
}

static op_t create_unary(optype_t kind, operand_t expr) {
    op_t op = { kind, { .expr = expr } };
    return op;
}

static op_t create_binary(optype_t kind, operand_t lhs, operand_t rhs) {
    op_t op = { kind, { .lhs = lhs, .rhs = rhs } };
    return op;
}

static operand_t create_vreg(vreg_t vreg) {
    operand_t op = create_operand(VREG);
    op.vreg = vreg;
    return op;
}

/* TODO: IMM_L */
static operand_t create_imm_l(long long imm) {
    operand_t op = create_operand(IMM_L);
    op.imm_l = imm;
    return op;
}

static operand_t create_imm_i(int imm) {
    operand_t op = create_operand(IMM_I);
    op.imm_i = imm;
    return op;
}

static operand_t create_imm_digit(node_t *digit) {
    type_t *type = digit->typeof;
    ASSERT(type->kind == BUILTIN);
    ASSERT(type->builtin.real == INTEGER);

    switch (type->builtin.width) {
    case 4: return create_imm_i(digit->digit);
    case 8: return create_imm_l(digit->digit);
    default:
        reportf("type->builtin.width = %zu", type->builtin.width);
        return create_operand(NONE);
    }
}

static operand_t create_bool(bool b) {
    operand_t op = create_operand(BIMM);
    op.bimm = b;
    return op;
}

static operand_t emit_node(flow_gen_t *gen, node_t *node);

static operand_t emit_digit(node_t *node) {
    return create_imm_digit(node);
}

static int get_unary(int op) {
    switch (op) {
    case ADD: return OP_ABS;
    case SUB: return OP_NEG;

    default:
        reportf("get_unary(op = %d)\n", op);
        return OP_EMPTY;
    }
}

static operand_t emit_unary(flow_gen_t *gen, node_t *node) {
    op_t op = create_unary(get_unary(node->unary.op),
        emit_node(gen, node->unary.expr)
    );

    return create_vreg(flow_gen_add(gen, op));
}

static int get_binary(int op) {
    switch (op) {
    case ADD: return OP_ADD;
    case SUB: return OP_SUB;
    case DIV: return OP_DIV;
    case MUL: return OP_MUL;
    case REM: return OP_REM;

    default:
        reportf("get_binop(op = %d)\n", op);
        return OP_EMPTY;
    }
}

static operand_t emit_binary(flow_gen_t *gen, node_t *node) {
    op_t op = create_binary(get_binary(node->binary.op),
        emit_node(gen, node->binary.lhs),
        emit_node(gen, node->binary.rhs)
    );

    return create_vreg(flow_gen_add(gen, op));
}

static operand_t emit_return(flow_gen_t *gen, node_t *node) {
    operand_t ret = !!node->expr
        ? emit_node(gen, node->expr)
        : create_operand(NONE);

    op_t op = create_unary(OP_RET, ret);
    
    return create_vreg(flow_gen_add(gen, op));
}

static operand_t emit_call(flow_gen_t *gen, node_t *node) {
    ASSERT(node->expr->type == AST_IDENT);
    ASSERT(node->expr->text != NULL);

    op_t op = create_unary(OP_CALL, create_name(node->expr->text));

    return create_vreg(flow_gen_add(gen, op));
}

static void emit_stmts(flow_gen_t *gen, node_t *node) {
    for (size_t i = 0; i < node->stmts->len; i++) {
        emit_node(gen, node->stmts->data + i);
    }
}

static operand_t emit_bool(node_t *node) {
    return create_bool(node->b);
}

static operand_t emit_var(flow_gen_t *gen, node_t *node) {
    operand_t init = emit_node(gen, node->var.init);
    add_var(gen, node->var.name, init);
    return init;
}

static operand_t emit_node(flow_gen_t *gen, node_t *node) {
    /* TODO: we need to figure out when to cast */
    switch (node->type) {
    case AST_DIGIT: return emit_digit(node);
    case AST_BOOL: return emit_bool(node);
    case AST_UNARY: return emit_unary(gen, node);
    case AST_BINARY: return emit_binary(gen, node);
    case AST_RETURN: return emit_return(gen, node);
    case AST_CALL: return emit_call(gen, node);
    case AST_STMTS: 
        emit_stmts(gen, node); 
        return create_operand(NONE);
    case AST_VAR: return emit_var(gen, node);

    case AST_IDENT:
        return get_var(gen, node->text);
        
    case AST_CAST:
    case AST_TYPENAME:
    case AST_FUNC:
        reportf("emit_node(node->type = %d)", node->type);
        return create_operand(NONE);
    }

    reportf("unreachable");
    return create_operand(NONE);
}

static opkind_t get_result_type(char *name) {
    if (strcmp(name, "void") == 0)
        return NONE;
    else if (strcmp(name, "long") == 0)
        return IMM_L;
    else if (strcmp(name, "bool") == 0)
        return BIMM;
    else if (strcmp(name, "int") == 0)
        return IMM_I;

    reportf("get_result_type(%s)", name);
    return NONE;
}

static flow_t transform_flow(node_t *node) {
    flow_t flow = flow_new(node->func.name, 
        get_result_type(node->func.result->text), 
        32
    );
    
    flow_gen_t gen = flowgen_new(&flow);

    emit_node(&gen, node->func.body);

    return flow;
}

unit_t transform_ast(const char *name, nodes_t *nodes) {
    unit_t unit = unit_new(name, 4);
    for (size_t i = 0; i < nodes->len; i++) {
        flow_t flow = transform_flow(nodes->data + i);
        unit_add(&unit, flow);
    }
    return unit;
}
