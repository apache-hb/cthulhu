#include "ir.h"
#include "bison.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cthulhu/util/report.h"

static unit_t unit_new(const char *name, size_t size) {
    unit_t unit = { name, malloc(sizeof(flow_t) * size), size, 0 };
    return unit;
}

static void unit_add(unit_t *unit, flow_t flow) {
    ENSURE_SIZE(unit->flows, unit->len, unit->size, sizeof(flow_t), 4);
    unit->flows[unit->len++] = flow;
}

static flow_t flow_new(const char *name, size_t size) {
    flow_t flow = { name, malloc(sizeof(op_t) * size), size, 0 };
    return flow;
}

static size_t flow_add(flow_t *flow, op_t op) {
    ENSURE_SIZE(flow->ops, flow->len, flow->size, sizeof(op_t), 32);
    flow->ops[flow->len] = op;
    return flow->len++;
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
    operand_t op = { VREG, { .vreg = vreg } };
    return op;
}

static operand_t create_imm(int64_t imm) {
    operand_t op = { IMM, { .imm = imm } };
    return op;
}

#if 0
static bool node_has_effects(node_t *node) {
    switch (node->type) {
    case AST_DIGIT: 
        return false;
    case AST_UNARY: 
        return node_has_effects(node->unary.expr);
    case AST_BINARY: 
        return node_has_effects(node->binary.lhs) 
            || node_has_effects(node->binary.rhs);
    case AST_TERNARY:
        return node_has_effects(node->ternary.cond)
            || node_has_effects(node->ternary.lhs)
            || node_has_effects(node->ternary.rhs);
    case AST_RETURN:
        return node_has_effects(node->expr);
    }
}

static bool node_eval_const(node_t *node) {
    switch (node->type) {
    case AST_DIGIT: return !!node->digit;
    }
}
#endif

static size_t emit_node(flow_t *flow, node_t *node);

static size_t emit_digit(flow_t *flow, node_t *node) {
    op_t op = { 
        .kind = OP_VALUE, 
        { .expr = create_imm(node->digit) } 
    };
    return flow_add(flow, op);
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

static size_t emit_unary(flow_t *flow, node_t *node) {
    op_t op = create_unary(get_unary(node->unary.op),
        create_vreg(emit_node(flow, node->unary.expr))
    );

    return flow_add(flow, op);
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

static size_t emit_binary(flow_t *flow, node_t *node) {
    op_t op = create_binary(get_binary(node->binary.op),
        create_vreg(emit_node(flow, node->binary.lhs)),
        create_vreg(emit_node(flow, node->binary.rhs))
    );

    return flow_add(flow, op);
}

static size_t emit_return(flow_t *flow, node_t *node) {
    op_t op = create_unary(OP_RET,
        create_vreg(emit_node(flow, node->expr))
    );
    
    return flow_add(flow, op);
}

static branch_t create_branch(size_t block, operand_t val) {
    branch_t branch = { block, val };
    return branch;
}

static size_t create_phi(flow_t *flow, branch_t a, branch_t b) {
    op_t op = { OP_PHI, { } };

    branch_t *branches = malloc(sizeof(branch_t) * 2);
    branches[0] = a;
    branches[1] = b;

    op.branches = branches;
    op.size = 2;
    op.len = 2;

    return flow_add(flow, op);
}

static size_t emit_block(flow_t *flow) {
    op_t op = { OP_BLOCK, { } };
    return flow_add(flow, op);
}

static size_t emit_cond(flow_t *flow, operand_t cond) {
    op_t op = { OP_COND, { .cond = cond, .block = SIZE_MAX } };
    return flow_add(flow, op);
}

static size_t emit_jmp(flow_t *flow) {
    op_t op = { OP_JMP, { } };
    return flow_add(flow, op);
}

static op_t *flow_at(flow_t *flow, size_t idx) {
    return flow->ops + idx;
}

static void fixup_cond(flow_t *flow, size_t idx, size_t block, size_t other) {
    op_t *op = flow_at(flow, idx);

    op->block = block;
    op->other = other;
}

/**
 * val = cond ? lhs : rhs
 * 
 *   %0 = cond
 *   jmp .true if %0
 * .true:
 *   %1 = lhs
 *   jmp .end
 * .false:
 *   %2 = rhs
 * .end:
 *   ; when the true path is taken %3 = %1
 *   ; when the false path is taken %3 = %2
 *   %3 = phi [ %1 when .true ], [ %2 when .false ]
 */

static size_t emit_ternary(flow_t *flow, node_t *node) {
    size_t cond = emit_node(flow, node->ternary.cond);
    size_t branch = emit_cond(flow, create_vreg(cond));

    size_t true_block = emit_block(flow);
    size_t true_path = emit_node(flow, node->ternary.lhs);

    size_t escape = emit_jmp(flow);

    size_t false_block = emit_block(flow);
    size_t false_path = emit_node(flow, node->ternary.rhs);

    size_t end_block = emit_block(flow);

    fixup_cond(flow, branch, true_block, false_block);
    flow_at(flow, escape)->label = end_block;

    return create_phi(flow,
        create_branch(true_block, create_vreg(true_path)),
        create_branch(false_block, create_vreg(false_path))
    );
}

static size_t emit_node(flow_t *flow, node_t *node) {
    switch (node->type) {
    case AST_DIGIT: return emit_digit(flow, node);
    case AST_UNARY: return emit_unary(flow, node);
    case AST_BINARY: return emit_binary(flow, node);
    case AST_RETURN: return emit_return(flow, node);
    case AST_TERNARY: return emit_ternary(flow, node);

    default:
        reportf("emit_node(node->type = %d)\n", node->type);
        return 0;
    }
}

static flow_t transform_flow(node_t *node) {
    flow_t flow = flow_new("main", 32);
    
    emit_node(&flow, node);

    return flow;
}

unit_t transform_ast(nodes_t *nodes) {
    unit_t unit = unit_new("root", 4);
    for (size_t i = 0; i < nodes->len; i++) {
        flow_t flow = transform_flow(nodes->data + i);
        unit_add(&unit, flow);
    }
    return unit;
}

static bool unit_apply(unit_t *unit, bool(*func)(flow_t*)) {
    bool dirty = false;

    for (size_t i = 0; i < unit->len; i++) {
        dirty = dirty || func(unit->flows + i);
    }

    return dirty;
}

static bool get_op(flow_t *flow, operand_t *op) {
    if (op->kind == IMM)
        return false;

    op_t *it = flow_at(flow, op->vreg);

    if (it->kind == OP_VALUE) {
        *op = it->expr;
        return true;
    }

    return false;
}

static bool fold_ret(flow_t *flow, op_t *op) {
    return get_op(flow, &op->expr);
}

static bool fold_unary(int64_t(*func)(int64_t), flow_t *flow, op_t *op) {
    bool dirty = get_op(flow, &op->expr);
    if (op->expr.kind == IMM) {
        *op = create_unary(OP_VALUE, 
            create_imm(func(op->expr.imm))
        );
        dirty = true;
    }
    return dirty;
}

static int64_t fold_abs(int64_t it) {
    return llabs(it);
}

static int64_t fold_neg(int64_t it) {
    return it * -1;
}

static bool flow_fold_ir(flow_t *flow) {
    bool dirty = false;
    
    for (size_t i = 0; i < flow->len; i++) {
        op_t *op = flow_at(flow, i);

        switch (op->kind) {
        case OP_EMPTY: case OP_VALUE:
            break;

        case OP_ABS:
            dirty = dirty || fold_unary(fold_abs, flow, op); 
            break;

        case OP_NEG:
            dirty = dirty || fold_unary(fold_neg, flow, op); 
            break;

        case OP_RET:
            dirty = dirty || fold_ret(flow, op);
            break;

        default:
            reportf("flow_fold_ir([%zu] = %d)", i, op->kind);
            break;
        }
    }

    return dirty;
}

static bool flow_raise_ir(flow_t *flow) {
    (void)flow;
    return false;
}

static bool flow_reduce_ir(flow_t *flow) {
    (void)flow;
    return false;
}

bool fold_ir(unit_t *unit) {
    return unit_apply(unit, flow_fold_ir);
}

bool raise_ir(unit_t *unit) {
    return unit_apply(unit, flow_raise_ir);
}

bool reduce_ir(unit_t *unit) {
    return unit_apply(unit, flow_reduce_ir);
}
