#include "cthulhu/ssa/ssa.h"

static operand_t operand_new(operand_type_t type) {
    operand_t operand = { .type = type };
    return operand;
}

static operand_t operand_vreg(size_t vreg) {
    operand_t operand = operand_new(OPERAND_VREG);
    operand.vreg = vreg;
    return operand;
}

static operand_t operand_digit(const type_t *type, const node_t *node, const mpz_t value) {
    operand_t operand = operand_new(OPERAND_VALUE);
    operand.value = value_digit(type, node, value);
    return operand;
}

static operand_t operand_string(const type_t *type, const node_t *node, const char *string) {
    operand_t operand = operand_new(OPERAND_VALUE);
    operand.value = value_string(type, node, string);
    return operand;
}

static operand_t operand_block(block_t *block) {
    operand_t operand = operand_new(OPERAND_BLOCK);
    operand.block = block;
    return operand;
}

static operand_t operand_empty(void) {
    return operand_new(OPERAND_EMPTY);
}



static void ssa_begin(ssa_t *ssa, size_t length) {
    ssa->steps = ctu_malloc(sizeof(step_t) * length);
    ssa->length = length;
    ssa->used = 0;
}

static void ssa_end(ssa_t *ssa, block_t *block) {
    block->steps = ssa->steps;
    block->length = ssa->used;
}



static size_t push_step(ssa_t *ssa, step_t step) {
    if (ssa->used >= ssa->length) {
        ssa->length += 16;
        ssa->steps = ctu_realloc(ssa->steps, ssa->length * sizeof(step_t));
    }

    ssa->steps[ssa->used++] = step;
    return ssa->used - 1;
}

static operand_t add_vreg(ssa_t *ssa, step_t step) {
    size_t vreg = push_step(ssa, step);
    return operand_vreg(vreg);
}

static step_t new_step(step_type_t type, const node_t *node) {
    step_t step = {
        .type = type,
        .node = node
    };

    return step;
}

static operand_t emit_ssa(ssa_t *ssa, const hlir_t *hlir);

static operand_t emit_name(ssa_t *ssa, const hlir_t *hlir) {
    step_t step = new_step(OP_LOAD, hlir->node);
    step.value = emit_ssa(ssa, hlir->ident);
    return add_vreg(ssa, step);
}

static operand_t emit_digit(const hlir_t *hlir) {
    return operand_digit(hlir->type, hlir->node, hlir->digit);
}

static operand_t emit_string(const hlir_t *hlir) {
    return operand_string(hlir->type, hlir->node, hlir->string);
}

static operand_t emit_binary(ssa_t *ssa, const hlir_t *hlir) {
    operand_t lhs = emit_ssa(ssa, hlir->lhs);
    operand_t rhs = emit_ssa(ssa, hlir->rhs);

    step_t step = new_step(OP_BINARY, hlir->node);

    step.lhs = lhs;
    step.rhs = rhs;
    step.binary = hlir->binary;

    return add_vreg(ssa, step);
}

static operand_t emit_call(ssa_t *ssa, const hlir_t *hlir) {
    operand_t call = emit_ssa(ssa, hlir->call);

    vector_t *args = hlir->args;
    size_t len = vector_len(args);

    operand_t *operands = ctu_malloc(sizeof(operand_t) * len);
    for (size_t i = 0; i < len; i++) {
        operands[i] = emit_ssa(ssa, vector_get(args, i));
    }

    step_t step = new_step(OP_CALL, hlir->node);
    step.call = call;
    step.operands = operands;
    step.total = len;

    return add_vreg(ssa, step);
}

static operand_t emit_assign(ssa_t *ssa, const hlir_t *hlir) {
    operand_t dst = emit_ssa(ssa, hlir->dst);
    operand_t src = emit_ssa(ssa, hlir->src);

    step_t step = new_step(OP_STORE, hlir->node);
    step.dst = dst;
    step.src = src;

    return add_vreg(ssa, step);
}

static operand_t emit_function(ssa_t *ssa, const hlir_t *hlir) {
    block_t *block = map_ptr_get(ssa->blocks, hlir);
    if (block == NULL) {
        report(ssa->reports, INTERNAL, hlir->node, "block is NULL");
    }

    return operand_block(block);
}

static operand_t emit_value(ssa_t *ssa, const hlir_t *hlir) {
    block_t *block = map_ptr_get(ssa->blocks, hlir);
    if (block == NULL) {
        report(ssa->reports, INTERNAL, hlir->node, "block is NULL");
    }
    
    return operand_block(block);
}

static operand_t emit_ssa(ssa_t *ssa, const hlir_t *hlir) {
    switch (hlir->kind) {
    case HLIR_NAME: return emit_name(ssa, hlir);
    case HLIR_DIGIT: return emit_digit(hlir);
    case HLIR_STRING: return emit_string(hlir);
    case HLIR_BINARY: return emit_binary(ssa, hlir);
    case HLIR_CALL: return emit_call(ssa, hlir);
    case HLIR_FUNCTION: return emit_function(ssa, hlir);
    case HLIR_ASSIGN: return emit_assign(ssa, hlir);
    case HLIR_VALUE: return emit_value(ssa, hlir);
    default: 
        report(ssa->reports, INTERNAL, hlir->node, "unexpected hlir kind %d", hlir->kind);
        return operand_empty();
    }
}

static void compile_global(ssa_t *ssa, block_t *block, const hlir_t *hlir) {
    ssa_begin(ssa, 16);

    step_t ret = new_step(OP_RETURN, hlir->node);
    ret.value = emit_ssa(ssa, hlir->value);
    push_step(ssa, ret);

    ssa_end(ssa, block);
}

static void compile_function(ssa_t *ssa, block_t *block, const hlir_t *hlir) {
    ssa_begin(ssa, 16);

    for (size_t i = 0; i < vector_len(hlir->body); i++) {
        const hlir_t *stmt = vector_get(hlir->body, i);
        emit_ssa(ssa, stmt);
    }
    step_t ret = new_step(OP_RETURN, hlir->node);
    ret.value = operand_empty();
    push_step(ssa, ret);

    ssa_end(ssa, block);
}

block_t *build_global(ssa_t *ssa, const hlir_t *hlir) {
    block_t *block = ctu_malloc(sizeof(block_t));
    block->name = hlir->name;

    compile_global(ssa, block, hlir);

    return block;
}

static block_t *begin_block(const hlir_t *hlir) {
    block_t *block = ctu_malloc(sizeof(block_t));
    block->name = hlir->name;
    return block;
}

module_t *build_module(ssa_t *ssa, const hlir_t *hlir) {
    size_t nglobals = vector_len(hlir->globals);
    size_t nfunctions = vector_len(hlir->defines);
    size_t nimports = vector_len(hlir->imports);

    module_t *mod = ctu_malloc(sizeof(module_t));

    mod->name = hlir->mod;
    mod->source = hlir->node->scan;
    mod->globals = vector_of(nglobals);
    mod->functions = vector_of(nfunctions);

    for (size_t i = 0; i < nimports; i++) {
        hlir_t *obj = vector_get(hlir->imports, i);
        block_t *block = begin_block(obj);
        map_ptr_set(ssa->blocks, obj, block);
    }

    for (size_t i = 0; i < nglobals; i++) {
        hlir_t *global = vector_get(hlir->globals, i);
        block_t *block = begin_block(global);
        map_ptr_set(ssa->blocks, global, block);
        vector_set(mod->globals, i, block);
    }

    for (size_t i = 0; i < nfunctions; i++) {
        hlir_t *function = vector_get(hlir->defines, i);
        block_t *block = begin_block(function);
        map_ptr_set(ssa->blocks, function, block);
        vector_set(mod->functions, i, block);
    }

    for (size_t i = 0; i < nglobals; i++) {
        hlir_t *global = vector_get(hlir->globals, i);
        block_t *block = vector_get(mod->globals, i);
        compile_global(ssa, block, global);
    }

    for (size_t i = 0; i < nfunctions; i++) {
        hlir_t *function = vector_get(hlir->defines, i);
        block_t *block = vector_get(mod->functions, i);
        compile_function(ssa, block, function);
    }

    return mod;
}

ssa_t *build_ssa(reports_t *reports) {
    ssa_t *ssa = ctu_malloc(sizeof(ssa_t));

    ssa->reports = reports;
    ssa->blocks = optimal_map(1024);
    
    return ssa;
}
