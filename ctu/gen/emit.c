#include "emit.h"

#include "ctu/util/util.h"
#include "ctu/util/report.h"

typedef struct {
    module_t *mod;
    block_t *block;
    reports_t *reports;
} context_t;

static size_t push_step(block_t *block, step_t step) {
    if (block->len + 1 >= block->size) {
        block->size += 16;
        block->steps = ctu_realloc(block->steps, block->size * sizeof(step_t));
    }

    block->steps[block->len] = step;
    return block->len++;
}

static operand_t new_operand(optype_t kind) {
    operand_t operand;
    operand.kind = kind;
    return operand;
}

static operand_t new_imm(value_t *value) {
    operand_t operand = new_operand(IMM);
    operand.imm = value;
    return operand;
}

static operand_t new_address(block_t *block) {
    operand_t operand = new_operand(ADDRESS);
    operand.block = block;
    return operand;
}

static value_t *value_zero(void) {
    return value_int(type_digit(false, TY_SIZE), 0);
}

static step_t step_with_type(opcode_t op, node_t *node, const type_t *type) {
    step_t step = {
        .opcode = op,
        .node = node,
        .type = type
    };

    return step;
}

static step_t step_of(opcode_t op, lir_t *lir) {
    return step_with_type(op, lir->node, lir_type(lir));
}

static step_t empty_step(opcode_t op) {
    return step_with_type(op, NULL, NULL);
}

static step_t *get_step(context_t ctx, operand_t op) {
    if (op.kind == LABEL) {
        return &ctx.block->steps[op.label];
    }

    assert2(ctx.reports, "get-step invalid kind %d", op.kind);

    return NULL;
}

static operand_t add_step(context_t ctx, step_t step) {
    operand_t op = new_operand(VREG);
    op.vreg = push_step(ctx.block, step);
    return op;
}

static operand_t add_label(context_t ctx, step_t step) {
    operand_t op = new_operand(LABEL);
    op.label = push_step(ctx.block, step);
    return op;
}

static operand_t emit_lir(context_t ctx, lir_t *lir);

static operand_t build_return(context_t ctx, lir_t *lir, operand_t op) {
    step_t step = step_of(OP_RETURN, lir);
    step.operand = op;
    return add_step(ctx, step);
}

static block_t *init_block(lir_t *decl, const type_t *type) {
    block_t *block = NEW(block_t);
    
    block->name = decl->name;
    block->result = type;

    block->len = 0;
    block->size = 16;
    block->steps = NEW_ARRAY(step_t, block->size);

    return block;
}

static block_t *block_declare(lir_t *lir) {
    block_t *block = init_block(lir, lir_type(lir));
    lir->data = block;
    return block;
}

static block_t *compile_block(context_t ctx, lir_t *decl) {
    block_t *block = block_declare(decl);
    build_return(ctx, decl->init, emit_lir(ctx, decl->init));
    vector_push(&ctx.mod->vars, block);
    return block;
}

static void build_block(reports_t *reports, module_t *mod, block_t *block, lir_t *body) {
    context_t ctx = { mod, block, reports };

    if (body != NULL) {
        operand_t op = emit_lir(ctx, body);
        build_return(ctx, body, op);
    }
}

static operand_t emit_unary(context_t ctx, lir_t *lir) {
    operand_t operand = emit_lir(ctx, lir->operand);
    step_t step = step_of(OP_UNARY, lir);
    step.unary = lir->unary;
    step.operand = operand;
    return add_step(ctx, step);
}

static operand_t emit_binary(context_t ctx, lir_t *lir) {
    step_t step = step_of(OP_BINARY, lir);
    step.binary = lir->binary;
    step.lhs = emit_lir(ctx, lir->lhs);
    step.rhs = emit_lir(ctx, lir->rhs);
    return add_step(ctx, step);
}

static operand_t emit_digit(lir_t *lir) {
    return new_imm(value_digit(lir_type(lir), lir->digit));
}

static operand_t emit_value(context_t ctx, lir_t *lir) {
    block_t *other = lir->data;

    if (other == NULL) {
        other = compile_block(ctx, lir);
        lir->data = other;
    }

    operand_t op = new_address(other);

    return op;
}

static operand_t emit_define(context_t ctx, lir_t *lir) {
    block_t *other = lir->data;

    if (other == NULL) {
        other = compile_block(ctx, lir);
        lir->data = other;
    }

    operand_t op = new_address(other);
    
    return op;
}

static operand_t emit_stmts(context_t ctx, lir_t *lir) {
    size_t len = vector_len(lir->stmts);
    for (size_t i = 0; i < len; i++) {
        lir_t *stmt = vector_get(lir->stmts, i);
        emit_lir(ctx, stmt);
    }
    return new_operand(EMPTY);
}

static operand_t emit_assign(context_t ctx, lir_t *lir) {
    operand_t dst = emit_lir(ctx, lir->dst);
    operand_t src = emit_lir(ctx, lir->src);

    step_t step = step_of(OP_STORE, lir->dst);
    step.dst = dst;
    step.src = src;
    step.offset = new_imm(value_zero());
    return add_step(ctx, step);
}

static operand_t add_block(context_t ctx, lir_t *lir) {
    if (lir == NULL) {
        return add_label(ctx, empty_step(OP_BLOCK));
    }

    operand_t block = add_label(ctx, step_of(OP_BLOCK, lir));
    emit_lir(ctx, lir);
    return block;
}

static operand_t emit_while(context_t ctx, lir_t *lir) {
    operand_t begin = add_label(ctx, step_of(OP_BLOCK, lir));
    
    step_t step = step_of(OP_BRANCH, lir);
    step.cond = emit_lir(ctx, lir->cond);
    operand_t branch = add_label(ctx, step);

    operand_t body = add_block(ctx, lir->then);

    operand_t end = add_label(ctx, step_of(OP_BLOCK, lir));

    step_t *it = get_step(ctx, branch);
    it->label = body;
    it->other = end;

    return begin;
}

static operand_t emit_branch(context_t ctx, lir_t *lir) {
    step_t step = step_of(OP_BRANCH, lir);
    step.cond = emit_lir(ctx, lir->cond);
    operand_t branch = add_label(ctx, step);

    operand_t yes = add_block(ctx, lir->then);

    operand_t no = add_block(ctx, lir->other);

    step_t *it = get_step(ctx, branch);
    it->label = yes;
    it->other = no;

    return branch;
}

static operand_t emit_name(context_t ctx, lir_t *lir) {
    step_t step = step_with_type(OP_LOAD, lir->node, lir_type(lir->it));
    step.src = emit_lir(ctx, lir->it);
    step.offset = new_imm(value_zero());
    return add_step(ctx, step);
}

static operand_t emit_call(context_t ctx, lir_t *lir) {
    size_t len = vector_len(lir->args);
    operand_t *args = NEW_ARRAY(operand_t, len);
    
    for (size_t i = 0; i < len; i++) {
        lir_t *arg = vector_get(lir->args, i);
        args[i] = emit_lir(ctx, arg);
    }

    step_t step = step_of(OP_CALL, lir);
    step.func = emit_lir(ctx, lir->func);
    step.args = args;
    step.len = len;

    return add_step(ctx, step);
}

static operand_t emit_lir(context_t ctx, lir_t *lir) {
    switch (lir->leaf) {
    case LIR_UNARY: return emit_unary(ctx, lir);
    case LIR_BINARY: return emit_binary(ctx, lir);
    case LIR_DIGIT: return emit_digit(lir);
    case LIR_VALUE: return emit_value(ctx, lir);
    case LIR_STMTS: return emit_stmts(ctx, lir);
    case LIR_ASSIGN: return emit_assign(ctx, lir);
    case LIR_WHILE: return emit_while(ctx, lir);
    case LIR_BRANCH: return emit_branch(ctx, lir);
    case LIR_NAME: return emit_name(ctx, lir);
    case LIR_CALL: return emit_call(ctx, lir);
    case LIR_DEFINE: return emit_define(ctx, lir);

    default:
        assert2(ctx.reports, "emit-lir unknown %d", lir->leaf);
        return new_operand(EMPTY);
    }
}


static module_t *init_module(vector_t *vars, vector_t *funcs, const char *name) {
    module_t *mod = NEW(module_t);
    mod->name = name;
    mod->vars = vars;
    mod->funcs = funcs;
    return mod;
}

module_t *module_build(reports_t *reports, lir_t *root) {
    vector_t *vars = root->vars;
    size_t nvars = vector_len(vars);

    vector_t *funcs = root->funcs;
    size_t nfuncs = vector_len(funcs);

    vector_t *varblocks = vector_of(nvars);
    vector_t *funcblocks = vector_of(nfuncs);

    module_t *mod = init_module(varblocks, funcblocks, root->node->scan->path);

    for (size_t i = 0; i < nvars; i++) {
        lir_t *var = vector_get(vars, i);
        vector_set(varblocks, i, block_declare(var));
    }

    for (size_t i = 0; i < nfuncs; i++) {
        lir_t *func = vector_get(funcs, i);
        vector_set(funcblocks, i, block_declare(func));
    }

    for (size_t i = 0; i < nvars; i++) {
        lir_t *var = vector_get(vars, i);
        block_t *block = vector_get(varblocks, i);
        build_block(reports, mod, block, var->init);
    }

    for (size_t i = 0; i < nfuncs; i++) {
        lir_t *func = vector_get(funcs, i);
        block_t *block = vector_get(funcblocks, i);
        build_block(reports, mod, block, func->body);
    }

    return mod;
}
