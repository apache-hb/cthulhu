#include "emit.h"

#include "ctu/util/util.h"
#include "ctu/util/report.h"

typedef struct {
    module_t *mod;
    block_t *block;
    map_t *blocks;
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

static value_t *new_value(type_t *type) {
    value_t *value = ctu_malloc(sizeof(value_t));
    value->type = type;
    return value;
}

static value_t *new_digit(type_t *type, mpz_t digit) {
    value_t *value = new_value(type);
    mpz_init_set(value->digit, digit);
    return value;
}

static value_t *new_zero() {
    value_t *value = new_value(type_digit(false, TY_SIZE));
    mpz_init_set_ui(value->digit, 0);
    return value;
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

static step_t step_of(opcode_t op, lir_t *lir) {
    step_t step = {
        .opcode = op,
        .node = lir->node,
        .type = lir->type
    };

    return step;
}

static operand_t add_step(context_t ctx, step_t step) {
    operand_t op = new_operand(VREG);
    op.vreg = push_step(ctx.block, step);
    return op;
}

static operand_t emit_lir(context_t ctx, lir_t *lir);

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
    return new_imm(new_digit(lir->type, lir->digit));
}

static operand_t emit_value(context_t ctx, lir_t *lir) {
    block_t *other = map_get(ctx.blocks, lir->name);

    operand_t op = new_address(other);
    step_t step = step_of(OP_LOAD, lir);
    step.src = op;
    step.offset = new_imm(new_zero());

    return add_step(ctx, step);
}

static operand_t build_return(context_t ctx, lir_t *lir, operand_t op) {
    step_t step = step_of(OP_RETURN, lir);
    step.operand = op;
    return add_step(ctx, step);
}

static operand_t emit_lir(context_t ctx, lir_t *lir) {
    switch (lir->leaf) {
    case LIR_UNARY: return emit_unary(ctx, lir);
    case LIR_BINARY: return emit_binary(ctx, lir);
    case LIR_DIGIT: return emit_digit(lir);
    case LIR_VALUE: return emit_value(ctx, lir);

    default:
        assert("emit-lir unknown %d", lir->leaf);
        return new_operand(EMPTY);
    }
}

static block_t *init_block(lir_t *decl, block_type_t kind, type_t *type) {
    block_t *block = ctu_malloc(sizeof(block_t));
    
    block->name = decl->name;
    block->result = type;
    block->type = kind;

    block->len = 0;
    block->size = 16;
    block->steps = ctu_malloc(sizeof(step_t) * block->size);

    return block;
}

static void build_block(module_t *mod, block_t *block, map_t *lookup, lir_t *body) {
    context_t ctx = { mod, block, lookup };

    if (body != NULL) {
        operand_t op = emit_lir(ctx, body);
        build_return(ctx, body, op);
    }
}

static module_t *init_module(vector_t *blocks, const char *name) {
    module_t *mod = ctu_malloc(sizeof(module_t));
    mod->name = name;
    mod->blocks = blocks;
    return mod;
}

module_t *module_build(lir_t *root) {
    vector_t *vars = root->vars;
    size_t len = vector_len(vars);

    vector_t *blocks = vector_of(len);
    module_t *mod = init_module(blocks, root->node->scan->path);

    map_t *lookup = map_new(32);

    for (size_t i = 0; i < len; i++) {
        lir_t *var = vector_get(vars, i);
        block_t *block = init_block(var, BLOCK_VALUE, var->type);
        vector_set(blocks, i, block);
        map_set(lookup, var->name, block);
    }

    for (size_t i = 0; i < len; i++) {
        lir_t *var = vector_get(vars, i);
        block_t *block = vector_get(blocks, i);
        build_block(mod, block, lookup, var->init);
    }

    return mod;
}
