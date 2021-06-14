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

static op_t *flow_at(flow_t *flow, size_t idx) {
    return flow->ops + idx;
}

static size_t emit_op(flow_t *flow, optype_t kind) {
    op_t op = { kind, { } };
    return flow_add(flow, op);
}

static op_t create_unary(optype_t kind, operand_t expr) {
    op_t op = { kind, { .expr = expr } };
    return op;
}

static op_t create_binary(optype_t kind, operand_t lhs, operand_t rhs) {
    op_t op = { kind, { .lhs = lhs, .rhs = rhs } };
    return op;
}

static operand_t create_operand(opkind_t kind) {
    operand_t op = { kind, { }, SIZE_MAX };
    return op;
}

static operand_t create_vreg(vreg_t vreg) {
    operand_t op = create_operand(VREG);
    op.vreg = vreg;
    return op;
}

static operand_t create_imm(int64_t imm) {
    operand_t op = create_operand(IMM);
    op.imm = imm;
    return op;
}

static operand_t create_name(char *text) {
    ASSERT(text != NULL);

    operand_t op = create_operand(NAME);
    op.name = text;
    return op;
}

static operand_t create_phi_branch(operand_t value, vreg_t block) {
    value.block = block;
    return value;
}

static size_t emit_node(flow_t *flow, node_t *node);

static size_t emit_digit(flow_t *flow, node_t *node) {
    op_t op = create_unary(OP_VALUE, create_imm(node->digit));
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

static bool is_expr_pure(node_t *node) {
    switch (node->type) {
    case AST_DIGIT: return true;
    case AST_CALL: return false;
    case AST_UNARY: 
        return is_expr_pure(node->unary.expr);

    case AST_BINARY: 
        return is_expr_pure(node->binary.lhs) 
            && is_expr_pure(node->binary.rhs);

    case AST_TERNARY:
        return is_expr_pure(node->ternary.cond)
            && is_expr_pure(node->ternary.lhs)
            && is_expr_pure(node->ternary.rhs);

    default:
        reportf("is_expr_pure(node->type = %d)", node->type);
        return false;
    }
}

static int64_t const_eval_node(node_t *node);

static int64_t const_eval_binary(node_t *node) {
    ASSERT(node->type == AST_BINARY);

    int op = node->binary.op;
    int64_t lhs = const_eval_node(node->binary.lhs),
            rhs = const_eval_node(node->binary.rhs);

    switch (op) {
    case ADD: return lhs + rhs;
    case SUB: return lhs - rhs;
    case DIV: return lhs / rhs;
    case MUL: return lhs * rhs;
    case REM: return lhs % rhs;

    default:
        reportf("const_eval_binary(node->binary.op = %d)", op);
        return INT64_MAX;
    }
}

static int64_t const_eval_node(node_t *node) {
    switch (node->type) {
    case AST_DIGIT: return node->digit;
    case AST_BINARY: return const_eval_binary(node);

    default:
        reportf("const_eval_node(node->type = %d)", node->type);
        return INT64_MAX;
    }
}

static size_t emit_block(flow_t *flow) {
    return emit_op(flow, OP_BLOCK);
}

static size_t emit_branch(flow_t *flow, size_t cond) {
    op_t op = { OP_BRANCH, { 
        .cond = create_vreg(cond),
        .lhs = create_imm(SIZE_MAX),
        .rhs = create_imm(SIZE_MAX)
    }};
    return flow_add(flow, op);
}

static void fixup_branch(flow_t *flow, size_t idx, size_t lhs, size_t rhs) {
    op_t *op = flow_at(flow, idx);

    op->lhs = create_vreg(lhs);
    op->rhs = create_vreg(rhs);
}

static size_t emit_phi(flow_t *flow, operand_t lhs, operand_t rhs) {
    op_t op = create_binary(OP_PHI, lhs, rhs);
    return flow_add(flow, op);
}

static size_t emit_jump(flow_t *flow) {
    return emit_op(flow, OP_JUMP);
}

static size_t emit_ternary_impure(flow_t *flow, 
    node_t *cond,
    bool lhs_pure, node_t *lhs,
    bool rhs_pure, node_t *rhs
)
{
    /**
     * emit our needed prelude
     * we fixup the branch at the end of the function
     * 
     * .entry:
     *   %0 = cond
     *   branch .true_block when %0 else .false_block
     */
    size_t entry_block = emit_block(flow);
    size_t cond_node = emit_node(flow, cond);
    size_t branch_node = emit_branch(flow, cond_node);

    size_t true_block;
    size_t false_block;

    operand_t true_operand;
    operand_t false_operand;

    size_t true_escape = SIZE_MAX;
    size_t false_escape = SIZE_MAX;

    if (lhs_pure) {
        /** 
         * if the true path (lhs) is pure then emit
         * 
         * .entry:
         *   %0 = cond
         *   branch .exit when %0 else .false
         * .false:
         *   %1 = rhs
         *   jmp .exit
         * .exit:
         *   %2 = phi [ .entry lhs ], [ .false %1 ]
         */
        true_block = entry_block;
        false_block = emit_block(flow);
        false_operand = create_vreg(emit_node(flow, rhs));
        false_escape = emit_jump(flow);
        true_operand = create_imm(const_eval_node(lhs));
    } else if (rhs_pure) {
        /**
         * if the false path (rhs) is pure then emit
         * 
         * .entry:
         *   %0 = cond
         *   branch .true when %0 else .exit
         * .true:
         *   %1 = lhs
         *   jmp .exit
         * .exit:
         *   %2 = phi [ .entry rhs ], [ .true %1 ]
         */
        false_block = entry_block;
        true_block = emit_block(flow);
        true_operand = create_vreg(emit_node(flow, lhs));
        true_escape = emit_jump(flow);
        false_operand = create_imm(const_eval_node(rhs));
    } else {
        /**
         * if both sides are impure then we emit the full if
         * 
         * .entry:
         *   %0 = cond
         *   branch .true when %0 else .false
         * .true:
         *   %1 = lhs
         *   jmp .exit
         * .false:
         *   %2 = rhs
         *   jmp .exit
         * .exit:
         *   %3 = phi [ .true %1 ], [ .false %2 ]
         */
        true_block = emit_block(flow);
        true_operand = create_vreg(emit_node(flow, lhs));
        true_escape = emit_jump(flow);

        false_block = emit_block(flow);
        false_operand = create_vreg(emit_node(flow, rhs));
        false_escape = emit_jump(flow);
    }

    size_t exit_block = emit_block(flow);

    fixup_branch(flow, branch_node, 
        true_escape == SIZE_MAX ? exit_block : true_block, 
        false_escape == SIZE_MAX ? exit_block : false_block
    );

    if (true_escape != SIZE_MAX)
        flow_at(flow, true_escape)->label = exit_block;

    if (false_escape != SIZE_MAX)
        flow_at(flow, false_escape)->label = exit_block;

    size_t phi = emit_phi(flow, 
        create_phi_branch(true_operand, true_block),
        create_phi_branch(false_operand, false_block)
    );

    return phi;
}

/* emitting ternaries is over complicated here because of llvm limitations */
static size_t emit_ternary(flow_t *flow, node_t *node) {
    node_t *cond = node->ternary.cond;
    node_t *lhs = node->ternary.lhs;
    node_t *rhs = node->ternary.rhs;

    if (is_expr_pure(cond)) {
        if (const_eval_node(cond)) {
            return emit_node(flow, lhs);
        } else {
            return emit_node(flow, rhs);
        }
    }

    bool lhs_pure = is_expr_pure(lhs),
         rhs_pure = is_expr_pure(rhs);

    /**
     * once we get here we know the condition isnt pure 
     * 
     * we include a redundant `jmp .exit`
     * this is make llvm not explode
     */

    if (lhs_pure && rhs_pure) {
        /** 
         * if both sides are pure we can eval them and emit a select 
         * 
         * %0 = cond
         * %1 = select %0 lhs rhs
         */

        size_t cond_node = emit_node(flow, cond);

        int64_t lhs_val = const_eval_node(lhs),
                rhs_val = const_eval_node(rhs);

        op_t op = { OP_SELECT, {
            .cond = create_vreg(cond_node),
            .lhs = create_imm(lhs_val),
            .rhs = create_imm(rhs_val)
        }};

        return flow_add(flow, op);
    } else {
        return emit_ternary_impure(flow, cond, lhs_pure, lhs, rhs_pure, rhs);
    }

    ASSERT(false);
}

static size_t emit_call(flow_t *flow, node_t *node) {
    ASSERT(node->expr->type == AST_IDENT);
    ASSERT(node->expr->text != NULL);

    op_t op = create_unary(OP_CALL, create_name(node->expr->text));

    return flow_add(flow, op);
}

static size_t emit_node(flow_t *flow, node_t *node) {
    switch (node->type) {
    case AST_DIGIT: return emit_digit(flow, node);
    case AST_UNARY: return emit_unary(flow, node);
    case AST_BINARY: return emit_binary(flow, node);
    case AST_TERNARY: return emit_ternary(flow, node);
    case AST_RETURN: return emit_return(flow, node);
    case AST_CALL: return emit_call(flow, node);

    default:
        reportf("emit_node(node->type = %d)\n", node->type);
        return 0;
    }
}

static flow_t transform_flow(node_t *node) {
    flow_t flow = flow_new(node->func.name, 32);
    
    emit_node(&flow, node->func.body);

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
