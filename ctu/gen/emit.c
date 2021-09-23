#include "emit.h"

#include "ctu/util/util.h"
#include "ctu/util/report.h"
#include "ctu/util/str.h"

#include "eval.h"

#include <string.h>

/* https://itanium-cxx-abi.github.io/cxx-abi/abi.html */
static const char *mangle_type(const type_t *type) {
    if (is_digit(type)) { 
        digit_t digit = type->digit;
        switch (digit.kind) {
        case TY_CHAR: return digit.sign ? "c" : "a";
        case TY_SHORT: return digit.sign ? "s" : "t";
        case TY_INT: return digit.sign ? "i" : "j";
        case TY_LONG: return digit.sign ? "l" : "m";
        default: return NULL; /* TODO */
        }
    }

    if (is_void(type)) {
        return "v";
    }

    if (is_pointer(type)) {
        return format("P%s", mangle_type(type->ptr));
    }

    if (is_closure(type)) {
        size_t len = vector_len(type->args);

        /* if a closure has no arguments, it implicty has a void argument */
        if (len == 0) {
            return "v";
        }

        vector_t *args = vector_of(len);

        for (size_t i = 0; i < len; i++) {
            const char *it = mangle_type(vector_get(type->args, i));
            vector_set(args, i, (char*)it);
        }

        return strjoin("", args);
    }

    return NULL;
}

static char *mangle_name(const char *name, const type_t *type) {
    size_t len = strlen(name);
    const char *ty = mangle_type(type);
    return format("_Z%zu%s%s", len, name, ty);
}

static block_t *new_block(blocktype_t kind, 
                          const char *name, 
                          const node_t *node,
                          const type_t *type) {
    block_t *block = NEW(block_t);
    block->kind = kind;
    block->name = name;
    block->node = node;
    block->type = type;
    block->data = NULL;
    return block;
}

typedef struct {
    vector_t **strings;
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

static value_t *value_zero(void) {
    return value_int(type_digit(false, TY_SIZE), 0);
}

static step_t step_with_type(opcode_t op, const node_t *node, const type_t *type) {
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
    vreg_t vreg = push_step(ctx.block, step);
    return operand_vreg(vreg);
}

static operand_t add_label(context_t ctx, step_t step) {
    label_t label = push_step(ctx.block, step);
    return operand_label(label);
}

static operand_t emit_lir(context_t ctx, lir_t *lir);

static operand_t build_return(context_t ctx, lir_t *lir, operand_t op) {
    step_t step = step_of(OP_RETURN, lir);
    step.operand = op;
    return add_step(ctx, step);
}

static block_t *lir_named(const lir_t *lir) {
    return new_block(
        BLOCK_SYMBOL, 
        lir->name, 
        lir->node, 
        lir_type(lir)
    );
}
 
static block_t *init_block(lir_t *decl, const type_t *type) {
    /* itanium only mangles functions */
    const char *name = decl->name;
    if (lir_is(decl, LIR_DEFINE)) {
        name = decl->entry ?: mangle_name(decl->name, type);
    }
    
    block_t *block = new_block(
        BLOCK_DEFINE, 
        name, 
        decl->node, 
        type
    );

    if (lir_is(decl, LIR_DEFINE)) {
        vector_t *locals = decl->locals;
        vector_t *params = decl->params;
        block->locals = VECTOR_MAP(locals, lir_named);
        block->params = VECTOR_MAP(params, lir_named);
    } else {
        block->locals = NULL;
        block->params = NULL;
    }

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

static block_t *import_symbol(lir_t *lir) {
    block_t *block = new_block(
        BLOCK_SYMBOL, 
        lir->name, 
        lir->node,
        lir_type(lir)
    );
    lir->data = block;
    return block;
}

static void build_block(vector_t **strings, reports_t *reports, module_t *mod, block_t *block, lir_t *body) {
    context_t ctx = { strings, mod, block, reports };

    if (body != NULL) {
        operand_t op = emit_lir(ctx, body);
        build_return(ctx, body, op);
    }
}

static void build_define(vector_t **strings, reports_t *reports, module_t *mod, block_t *block, lir_t *define) {
    context_t ctx = { strings, mod, block, reports };

    vector_t *locals = define->locals;
    vector_t *params = define->params;

    for (size_t i = 0; i < vector_len(locals); i++) {
        block_declare(vector_get(locals, i));
    }

    for (size_t i = 0; i < vector_len(params); i++) {
        block_declare(vector_get(params, i));
    }

    lir_t *body = define->body;
    operand_t op = emit_lir(ctx, body);
    build_return(ctx, body, op);
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
    return operand_imm(value_digit(lir_type(lir), lir->digit));
}

static operand_t emit_value(const lir_t *lir) {
    return operand_address(lir->data);
}

static operand_t emit_define(const lir_t *lir) {
    return operand_address(lir->data);
}

static operand_t emit_stmts(context_t ctx, lir_t *lir) {
    size_t len = vector_len(lir->stmts);
    for (size_t i = 0; i < len; i++) {
        lir_t *stmt = vector_get(lir->stmts, i);
        emit_lir(ctx, stmt);
    }
    return operand_empty();
}

static operand_t emit_assign(context_t ctx, lir_t *lir) {
    operand_t dst = emit_lir(ctx, lir->dst);
    operand_t src = emit_lir(ctx, lir->src);

    step_t step = step_of(OP_STORE, lir->dst);
    step.dst = dst;
    step.src = src;
    step.offset = operand_imm(value_zero());
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

    step_t loop = step_of(OP_JMP, lir);
    loop.label = begin;
    add_step(ctx, loop);

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

    operand_t esc = add_label(ctx, step_with_type(OP_JMP, lir->node, NULL));

    operand_t no = add_block(ctx, lir->other);

    operand_t tail = lir->other == NULL ? no : add_block(ctx, NULL);

    step_t *it = get_step(ctx, branch);
    it->label = yes;
    it->other = no;

    step_t *jmp = get_step(ctx, esc);
    jmp->label = tail;

    return branch;
}

static operand_t emit_name(context_t ctx, lir_t *lir) {
    step_t step = step_with_type(OP_LOAD, lir->node, lir_type(lir->it));
    step.src = emit_lir(ctx, lir->it);
    step.offset = operand_imm(value_zero());
    return add_step(ctx, step);
}

static operand_t emit_call(context_t ctx, lir_t *lir) {
    size_t len = vector_len(lir->args);
    operand_t *args = NEW_ARRAY(operand_t, len);
    
    for (size_t i = 0; i < len; i++) {
        lir_t *arg = vector_get(lir->args, i);
        args[i] = emit_lir(ctx, arg);
    }

    operand_t func = emit_lir(ctx, lir->func);

    step_t step = step_with_type(OP_CALL, lir->node, lir_type(lir->func)->result);
    step.func = func;
    step.args = args;
    step.len = len;

    return add_step(ctx, step);
}

static operand_t emit_symbol(lir_t *lir) {
    return operand_address(lir->data);
}

static operand_t emit_string(context_t ctx, lir_t *lir) {
    block_t *str = new_block(
        BLOCK_STRING, 
        NULL, 
        lir->node,
        lir_type(lir)
    );
    str->idx = vector_len(*(ctx.strings));
    str->string = lir->str;
    vector_push(ctx.strings, str);
    return operand_address(str);
}

static operand_t emit_lir(context_t ctx, lir_t *lir) {
    switch (lir->leaf) {
    case LIR_UNARY: return emit_unary(ctx, lir);
    case LIR_BINARY: return emit_binary(ctx, lir);
    case LIR_DIGIT: return emit_digit(lir);
    case LIR_STRING: return emit_string(ctx, lir);
    case LIR_STMTS: return emit_stmts(ctx, lir);
    case LIR_ASSIGN: return emit_assign(ctx, lir);
    case LIR_WHILE: return emit_while(ctx, lir);
    case LIR_BRANCH: return emit_branch(ctx, lir);
    case LIR_NAME: return emit_name(ctx, lir);
    case LIR_CALL: return emit_call(ctx, lir);
    case LIR_SYMBOL: return emit_symbol(lir);
    case LIR_VALUE: return emit_value(lir);
    case LIR_DEFINE: return emit_define(lir);

    default:
        assert2(ctx.reports, "emit-lir unknown %d", lir->leaf);
        return operand_empty();
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

    vector_t *strings = vector_new(4);

    module_t *mod = init_module(varblocks, funcblocks, root->node->scan->path);

    for (size_t i = 0; i < nvars; i++) {
        lir_t *var = vector_get(vars, i);
        block_t *block = block_declare(var);
        vector_set(varblocks, i, block);
    }

    for (size_t i = 0; i < nfuncs; i++) {
        lir_t *func = vector_get(funcs, i);
        block_t *block = block_declare(func);
        vector_set(funcblocks, i, block);
    }

    vector_t *imports = root->imports;
    size_t nimports = vector_len(imports);
    vector_t *symbols = vector_of(nimports);
    for (size_t i = 0; i < nimports; i++) {
        lir_t *it = vector_get(imports, i);
        block_t *block = import_symbol(it);
        vector_set(symbols, i, block);
    }

    for (size_t i = 0; i < nvars; i++) {
        lir_t *var = vector_get(vars, i);
        block_t *block = vector_get(varblocks, i);
        build_block(&strings, reports, mod, block, var->init);
    }

    for (size_t i = 0; i < nfuncs; i++) {
        lir_t *func = vector_get(funcs, i);
        block_t *block = vector_get(funcblocks, i);
        build_define(&strings, reports, mod, block, func);
    }

    for (size_t i = 0; i < nvars; i++) {
        block_t *var = vector_get(varblocks, i);
        value_t *result = eval_block(reports, mod, var);

        var->value = result;
    }

    mod->imports = symbols;
    mod->strtab = strings;

    printf("imports %zu\n", vector_len(symbols));

    return mod;
}
