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

    /* TODO: is it safe to assume this? */
    return create_name(name);
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

static op_t *flow_at(flow_gen_t *gen, size_t idx) {
    return gen->flow->ops + idx;
}

static size_t emit_op(flow_gen_t *gen, optype_t kind) {
    op_t op = { kind, { .label = SIZE_MAX } };
    return flow_gen_add(gen, op);
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

static operand_t create_bool(bool b) {
    operand_t op = create_operand(BIMM);
    op.bimm = b;
    return op;
}

static operand_t create_phi_branch(operand_t value, vreg_t block) {
    value.block = block;
    return value;
}

static size_t emit_node(flow_gen_t *gen, node_t *node);

static size_t emit_digit(flow_gen_t *gen, node_t *node) {
    op_t op = create_unary(OP_VALUE, create_imm_i(node->digit));
    return flow_gen_add(gen, op);
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

static size_t emit_unary(flow_gen_t *gen, node_t *node) {
    op_t op = create_unary(get_unary(node->unary.op),
        create_vreg(emit_node(gen, node->unary.expr))
    );

    return flow_gen_add(gen, op);
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

static size_t emit_binary(flow_gen_t *gen, node_t *node) {
    op_t op = create_binary(get_binary(node->binary.op),
        create_vreg(emit_node(gen, node->binary.lhs)),
        create_vreg(emit_node(gen, node->binary.rhs))
    );

    return flow_gen_add(gen, op);
}

static size_t emit_return(flow_gen_t *gen, node_t *node) {
    operand_t ret = !!node->expr
        ? create_vreg(emit_node(gen, node->expr))
        : create_operand(NONE);

    op_t op = create_unary(OP_RET, ret);
    
    return flow_gen_add(gen, op);
}

static bool is_expr_pure(node_t *node) {
    switch (node->type) {
    case AST_BOOL:
    case AST_DIGIT: 
        return true;

    case AST_CALL: 
        return false;

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

static operand_t const_eval_node(node_t *node);

/* TODO: this is going to get ugly */
static operand_t const_eval_binary(node_t *node) {
    ASSERT(node->type == AST_BINARY);

    int op = node->binary.op;
    operand_t lhs = const_eval_node(node->binary.lhs),
            rhs = const_eval_node(node->binary.rhs);

    switch (op) {
    case ADD: return create_imm_i(lhs.imm_i + rhs.imm_i);
    case SUB: return create_imm_i(lhs.imm_i - rhs.imm_i);
    case DIV: return create_imm_i(lhs.imm_i / rhs.imm_i);
    case MUL: return create_imm_i(lhs.imm_i * rhs.imm_i);
    case REM: return create_imm_i(lhs.imm_i % rhs.imm_i);

    default:
        reportf("const_eval_binary(node->binary.op = %d)", op);
        return create_imm_i(INT_MAX);
    }
}

static operand_t const_eval_node(node_t *node) {
    switch (node->type) {
    case AST_DIGIT: return create_imm_i(node->digit);
    case AST_BOOL: return create_bool(node->b);
    case AST_BINARY: return const_eval_binary(node);

    default:
        reportf("const_eval_node(node->type = %d)", node->type);
        return create_operand(NONE);
    }
}

static size_t emit_block(flow_gen_t *gen) {
    return emit_op(gen, OP_BLOCK);
}

static size_t emit_branch(flow_gen_t *gen, size_t cond) {
    op_t op = { OP_BRANCH, { 
        .cond = create_vreg(cond),
        .lhs = create_imm_l(SIZE_MAX),
        .rhs = create_imm_l(SIZE_MAX)
    }};
    return flow_gen_add(gen, op);
}

static void fixup_branch(flow_gen_t *gen, size_t idx, size_t lhs, size_t rhs) {
    op_t *op = flow_at(gen, idx);

    op->lhs.block = lhs;
    op->rhs.block = rhs;
}

static size_t emit_phi(flow_gen_t *gen, operand_t lhs, operand_t rhs) {
    op_t op = create_binary(OP_PHI, lhs, rhs);
    return flow_gen_add(gen, op);
}

static size_t emit_jump(flow_gen_t *gen) {
    return emit_op(gen, OP_JUMP);
}

static size_t emit_ternary_impure(flow_gen_t *gen, 
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
    size_t entry_block = emit_block(gen);
    size_t cond_node = emit_node(gen, cond);
    size_t branch_node = emit_branch(gen, cond_node);

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
        false_block = emit_block(gen);
        false_operand = create_vreg(emit_node(gen, rhs));
        false_escape = emit_jump(gen);
        true_operand = const_eval_node(lhs);
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
        true_block = emit_block(gen);
        true_operand = create_vreg(emit_node(gen, lhs));
        true_escape = emit_jump(gen);
        false_operand = const_eval_node(rhs);
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
        true_block = emit_block(gen);
        true_operand = create_vreg(emit_node(gen, lhs));
        true_escape = emit_jump(gen);

        false_block = emit_block(gen);
        false_operand = create_vreg(emit_node(gen, rhs));
        false_escape = emit_jump(gen);
    }

    size_t exit_block = emit_block(gen);

    fixup_branch(gen, branch_node, 
        true_escape == SIZE_MAX ? exit_block : true_block, 
        false_escape == SIZE_MAX ? exit_block : false_block
    );

    if (true_escape != SIZE_MAX)
        flow_at(gen, true_escape)->label = exit_block;

    if (false_escape != SIZE_MAX)
        flow_at(gen, false_escape)->label = exit_block;

    size_t phi = emit_phi(gen, 
        create_phi_branch(true_operand, true_block),
        create_phi_branch(false_operand, false_block)
    );

    return phi;
}

/* emitting ternaries is over complicated here because of llvm limitations */
static size_t emit_ternary(flow_gen_t *gen, node_t *node) {
    node_t *cond = node->ternary.cond;
    node_t *lhs = node->ternary.lhs;
    node_t *rhs = node->ternary.rhs;

    if (is_expr_pure(cond)) {
        /* TODO: good lord this is terrible */
        if (const_eval_node(cond).bimm) {
            return emit_node(gen, lhs);
        } else {
            return emit_node(gen, rhs);
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

        size_t cond_node = emit_node(gen, cond);

        operand_t lhs_val = const_eval_node(lhs),
                rhs_val = const_eval_node(rhs);

        op_t op = { OP_SELECT, {
            .cond = create_vreg(cond_node),
            .lhs = lhs_val,
            .rhs = rhs_val
        }};

        return flow_gen_add(gen, op);
    } else {
        return emit_ternary_impure(gen, cond, lhs_pure, lhs, rhs_pure, rhs);
    }

    ASSERT(false);
}

static size_t emit_call(flow_gen_t *gen, node_t *node) {
    ASSERT(node->expr->type == AST_IDENT);
    ASSERT(node->expr->text != NULL);

    op_t op = create_unary(OP_CALL, create_name(node->expr->text));

    return flow_gen_add(gen, op);
}

static size_t emit_stmts(flow_gen_t *gen, node_t *node) {
    for (size_t i = 0; i < node->stmts->len; i++) {
        emit_node(gen, node->stmts->data + i);
    }

    /* the result of a statement should never be used */
    return SIZE_MAX;
}

static size_t emit_bool(flow_gen_t *gen, node_t *node) {
    return flow_gen_add(gen, create_unary(OP_VALUE, 
        create_bool(node->b)
    ));
}

static size_t emit_var(flow_gen_t *gen, node_t *node) {
    size_t init = emit_node(gen, node->var.init);
    add_var(gen, node->var.name, create_vreg(init));
    return init;
}

static size_t emit_ident(flow_gen_t *gen, node_t *node) {
    operand_t op = get_var(gen, node->text);
    if (op.kind == VREG) {
        return op.vreg;
    }
    reportf("failed to resolve ident %s", node->text);
    return SIZE_MAX;
}

static size_t emit_node(flow_gen_t *gen, node_t *node) {
    /* TODO: we need to figure out when to cast */
    switch (node->type) {
    case AST_DIGIT: return emit_digit(gen, node);
    case AST_BOOL: return emit_bool(gen, node);
    case AST_UNARY: return emit_unary(gen, node);
    case AST_BINARY: return emit_binary(gen, node);
    case AST_TERNARY: return emit_ternary(gen, node);
    case AST_RETURN: return emit_return(gen, node);
    case AST_CALL: return emit_call(gen, node);
    case AST_STMTS: return emit_stmts(gen, node);
    case AST_VAR: return emit_var(gen, node);
    case AST_IDENT: return emit_ident(gen, node);

    case AST_TYPENAME:
    case AST_FUNC:
        reportf("emit_node(node->type = %d)", node->type);
        return SIZE_MAX;
    }

    reportf("unreachable");
    return SIZE_MAX;
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
