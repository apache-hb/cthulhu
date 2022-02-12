#include "emit.h"

#include "cthulhu/util/util.h"
#include "cthulhu/util/report.h"
#include "cthulhu/util/str.h"
#include "eval.h"
#include <string.h>

static block_t *new_block(blocktype_t kind, 
                          const char *name, 
                          const node_t *node,
                          const type_t *type) {
    block_t *block = ctu_malloc(sizeof(block_t));
    block->kind = kind;
    block->name = name;
    block->node = node;
    block->type = type;
    block->data = NULL;

    return block;
}

static block_t *new_define(const char *name,
                           const node_t *node,
                           const type_t *type,
                           const attrib_t *attribs)
{
    block_t *block = new_block(BLOCK_DEFINE, name, node, type);
    block->attribs = attribs;
    block->value = NULL;
    return block;
}

typedef struct {
    vector_t *strings;
    module_t *mod;
    block_t *block;
    reports_t *reports;
    oplist_t *fixups;
    vector_t *types;
} context_t;

static void add_type(context_t *ctx, const type_t *type) {
    if (!is_builtin_type(type)) { return; }
    const type_t *actual = underlying_type(type);

    size_t len = vector_len(ctx->types);
    for (size_t i = 0; i < len; i++) {
        const type_t *at = vector_get(ctx->types, i);
        if (types_exact_equal(at, actual)) {
            return;
        }
    }
    vector_push(&ctx->types, (type_t*)actual);
}

static void add_fixup(context_t *ctx, lir_t *dst, operand_t op) {
    if (dst != NULL) {
        oplist_push(dst->data, op);
    } else {
        oplist_push(ctx->fixups, op);
    }
}

static oplist_t *get_fixups(context_t *ctx) {
    return ctx->fixups;
}

static void set_fixups(context_t *ctx, oplist_t *fixups) {
    ctx->fixups = fixups;
}

static size_t push_step(block_t *block, step_t step) {
    if (block->len + 1 >= block->size) {
        block->size += 16;
        block->steps = ctu_realloc(block->steps, block->size * sizeof(step_t));
    }

    block->steps[block->len] = step;
    return block->len++;
}

static step_t step_with_type(context_t *ctx, opcode_t op, const node_t *node, const type_t *type) {
    if (type != NULL) { add_type(ctx, type); }

    step_t step = {
        .opcode = op,
        .node = node,
        .type = type
    };

    return step;
}

static step_t step_of(context_t *ctx, opcode_t op, lir_t *lir) {
    return step_with_type(ctx, op, lir->node, lir_type(lir));
}

static step_t empty_step(context_t *ctx, opcode_t op) {
    return step_with_type(ctx, op, NULL, NULL);
}

static step_t *get_step(context_t *ctx, operand_t op) {
    if (op.kind == LABEL) {
        return &ctx->block->steps[op.label];
    }

    ctu_assert(ctx->reports, "get-step invalid kind %d", op.kind);

    return NULL;
}

static operand_t add_step(context_t *ctx, step_t step) {
    vreg_t vreg = push_step(ctx->block, step);
    return operand_vreg(vreg);
}

static operand_t add_label(context_t *ctx, step_t step) {
    label_t label = push_step(ctx->block, step);
    return operand_label(label);
}

static operand_t emit_lir(context_t *ctx, lir_t *lir);

static step_t new_return(context_t *ctx, const node_t *node, const type_t *type) {
    if (type == NULL) {
        report(ctx->reports, INTERNAL, node, "OP_RETURN type == NULL");
    }

    return step_with_type(ctx, OP_RETURN, node, type);
}

static operand_t build_return(context_t *ctx, const node_t *node, const type_t *type, operand_t op) {
    step_t step = new_return(ctx, node, type);
    step.operand = op;
    return add_step(ctx, step);
}

static const char *local_name(const lir_t *lir) {
    where_t where = lir->node->where;
    return format("%s%zu%zu", get_name(lir), where.first_line, where.first_column);
}

static const char *symbol_name(const lir_t *lir) {
    if (lir->attribs->mangle != NULL) {
        return lir->attribs->mangle;
    }

    if (has_name(lir)) {
        return get_name(lir);
    }

    node_t *node = lir->node;
    where_t where = node->where;
    return format("anon%ld_%ld", where.first_line, where.first_column);
}

static block_t *detailed_block(const char *name, const type_t *type, const attrib_t *attribs, const node_t *node) {
    block_t *block = new_define(name, node, type, attribs);

    block->locals = vector_new(4);
    block->len = 0;
    block->size = 16;
    block->steps = ctu_malloc(sizeof(step_t) * block->size);

    return block;
}

static block_t *init_block(const char *name, lir_t *decl, const type_t *type) {
    return detailed_block(name, type, decl->attribs, decl->node);
}

static block_t *block_declare(lir_t *lir) {
    block_t *block = init_block(symbol_name(lir), lir, lir_type(lir));
    lir->data = block;
    return block;
}

static block_t *import_symbol(lir_t *lir) {
    block_t *block = new_define(symbol_name(lir), lir->node, lir_type(lir), lir->attribs);
    lir->data = block;
    return block;
}

static block_t *local_declare(lir_t *lir) {
    block_t *block = init_block(local_name(lir), lir, lir_type(lir));
    lir->data = block;
    return block;
}

static context_t new_context(reports_t *reports, vector_t *strings, module_t *mod, block_t *block) {
    context_t ctx = {
        .strings = strings,
        .mod = mod,
        .block = block,
        .reports = reports,
        .fixups = oplist_new(4),
        .types = vector_new(4)
    };

    return ctx;
}

static context_t build_block(vector_t *strings, reports_t *reports, module_t *mod, block_t *block, lir_t *body) {
    context_t ctx = new_context(reports, strings, mod, block);

    if (body != NULL) {
        operand_t op = emit_lir(&ctx, body);
        build_return(&ctx, node_last_line(body->node), type_void(), op);
    } else {
        step_t step = new_return(&ctx, node_last_line(block->node), type_void());
        step.operand = operand_empty();
        add_step(&ctx, step);
    }
    
    return ctx;
}

static context_t build_define(vector_t *strings, reports_t *reports, module_t *mod, block_t *block, lir_t *define) {
    context_t ctx = new_context(reports, strings, mod, block);
    
    lir_t *body = define->body;
    operand_t op = emit_lir(&ctx, body);
    build_return(&ctx, node_last_line(body->node), type_void(), op);

    return ctx;
}

static operand_t compile_load(context_t *ctx, lir_t *lir, operand_t op, operand_t off) {
    step_t step = step_with_type(ctx, OP_LOAD, lir->node, lir_type(lir));
    step.src = op;
    step.offset = off;
    return add_step(ctx, step);
}

static operand_t emit_unary(context_t *ctx, lir_t *lir) {
    step_t step = step_of(ctx, OP_UNARY, lir);
    step.unary = lir->unary;
    step.operand = emit_lir(ctx, lir->operand);
    return add_step(ctx, step);
}

static operand_t emit_binary(context_t *ctx, lir_t *lir) {
    step_t step = step_of(ctx, OP_BINARY, lir);
    step.binary = lir->binary;
    step.lhs = emit_lir(ctx, lir->lhs);
    step.rhs = emit_lir(ctx, lir->rhs);
    return add_step(ctx, step);
}

static operand_t emit_digit(context_t *ctx, lir_t *lir) {
    return operand_imm(ctx->reports, value_digit(lir->node, lir_type(lir), lir->digit));
}

static operand_t emit_bool(context_t *ctx, lir_t *lir) {
    return operand_imm(ctx->reports, value_bool(lir->node, lir->boolean));
}

static operand_t emit_value(const lir_t *lir) {
    return operand_address(lir->data);
}

static operand_t emit_define(const lir_t *lir) {
    return operand_address(lir->data);
}

static operand_t emit_stmts(context_t *ctx, lir_t *lir) {
    size_t len = vector_len(lir->stmts);
    for (size_t i = 0; i < len; i++) {
        lir_t *stmt = vector_get(lir->stmts, i);
        emit_lir(ctx, stmt);
    }
    return operand_empty();
}

static operand_t compile_store(context_t *ctx, lir_t *lir, operand_t dst, operand_t src) {
    step_t step = step_of(ctx, OP_STORE, lir);
    step.dst = dst;
    step.src = src;
    return add_step(ctx, step);
}

static operand_t emit_assign(context_t *ctx, lir_t *lir) {
    lir_t *to = lir_is(lir->dst, LIR_UNARY) ? lir->dst->operand : lir->dst;
    operand_t dst = emit_lir(ctx, to);
    operand_t src = emit_lir(ctx, lir->src);

    return compile_store(ctx, lir->dst, dst, src);
}

static operand_t add_block(context_t *ctx, lir_t *lir) {
    if (lir == NULL) {
        return add_label(ctx, empty_step(ctx, OP_BLOCK));
    }

    operand_t block = add_label(ctx, step_of(ctx, OP_BLOCK, lir));
    emit_lir(ctx, lir);
    return block;
}

static operand_t emit_while(context_t *ctx, lir_t *lir) {
    operand_t begin = add_label(ctx, step_of(ctx, OP_BLOCK, lir));
    
    step_t step = step_of(ctx, OP_BRANCH, lir);
    step.cond = emit_lir(ctx, lir->cond);
    operand_t branch = add_label(ctx, step);

    oplist_t *stack = get_fixups(ctx);
    {
        oplist_t *temp = oplist_new(4);
        set_fixups(ctx, temp);
        lir->data = temp;

        operand_t body = add_block(ctx, lir->then);

        step_t loop = step_of(ctx, OP_JMP, lir);
        loop.label = begin;
        add_step(ctx, loop);

        /* create a vector of blocks that will later become fixups */
        operand_t end = add_label(ctx, step_of(ctx, OP_BLOCK, lir));

        step_t *it = get_step(ctx, branch);
        it->label = body;
        it->other = end;

        size_t len = oplist_len(temp);
        for (size_t i = 0; i < len; i++) {
            operand_t op = oplist_get(temp, i);
            step_t *fixup = get_step(ctx, op);
            fixup->label = end;
        }
    }
    set_fixups(ctx, stack);

    return begin;
}

static operand_t emit_branch(context_t *ctx, lir_t *lir) {
    step_t step = step_of(ctx, OP_BRANCH, lir);
    step.cond = emit_lir(ctx, lir->cond);
    operand_t branch = add_label(ctx, step);

    operand_t yes = add_block(ctx, lir->then);

    operand_t esc = add_label(ctx, step_with_type(ctx, OP_JMP, lir->node, NULL));

    operand_t no = add_block(ctx, lir->other);

    operand_t tail = lir->other == NULL ? no : add_block(ctx, NULL);

    step_t *it = get_step(ctx, branch);
    it->label = yes;
    it->other = no;

    step_t *jmp = get_step(ctx, esc);
    jmp->label = tail;

    return branch;
}

static operand_t emit_read(context_t *ctx, lir_t *lir) {
    operand_t offset = lir->offset == NULL
        ? operand_empty()
        : emit_lir(ctx, lir->offset);
    return compile_load(ctx, lir, emit_lir(ctx, lir->src), offset);
}

static operand_t emit_call(context_t *ctx, lir_t *lir) {
    size_t len = vector_len(lir->args);
    oplist_t *args = oplist_of(len);
    
    for (size_t i = 0; i < len; i++) {
        lir_t *arg = vector_get(lir->args, i);
        operand_t op = emit_lir(ctx, arg);
        oplist_set(args, i, op);
    }

    operand_t func = emit_lir(ctx, lir->func);

    step_t step = step_with_type(ctx, OP_CALL, lir->node, lir_type(lir->func)->result);
    step.func = func;
    step.args = args;

    return add_step(ctx, step);
}

static operand_t emit_string(context_t *ctx, lir_t *lir) {
    block_t *str = new_block(BLOCK_STRING, NULL, lir->node, lir_type(lir));
    str->idx = vector_len(ctx->strings);
    str->string = lir->str;
    vector_push(&ctx->strings, str);
    return operand_address(str);
}

static operand_t emit_forward(context_t *ctx, lir_t *lir) {
    report(ctx->reports, INTERNAL, lir->node, "forward `%s`", type_format(lir_type(lir)));
    return operand_empty();
}

static operand_t emit_poison(context_t *ctx, lir_t *lir) {
    report(ctx->reports, INTERNAL, lir->node, "poison `%s`", type_format(lir_type(lir)));
    return operand_empty();
}

static operand_t emit_return(context_t *ctx, lir_t *lir) {
    node_t *node = node_last_line(lir->node);
    if (lir->operand == NULL) {
        return build_return(ctx, node, type_void(), operand_empty());
    } else {
        return build_return(ctx, node, lir_type(lir->operand), emit_lir(ctx, lir->operand));
    }
}

static operand_t emit_param(lir_t *lir) {
    return operand_arg(lir->index);
}

static operand_t emit_break(context_t *ctx, lir_t *lir) {
    step_t step = step_of(ctx, OP_JMP, lir);
    step.label = operand_empty();
    operand_t jump = add_label(ctx, step);
    add_fixup(ctx, lir->loop, jump);
    return operand_empty();
}

static operand_t emit_sizeof(context_t *ctx, lir_t *lir) {
    step_t step = step_of(ctx, OP_BUILTIN, lir);
    step.builtin = BUILTIN_SIZEOF;
    step.target = lir->of;
    return add_step(ctx, step);
}

static operand_t emit_alignof(context_t *ctx, lir_t *lir) {
    step_t step = step_of(ctx, OP_BUILTIN, lir);
    step.builtin = BUILTIN_ALIGNOF;
    step.target = lir->of;
    return add_step(ctx, step);
}

static operand_t emit_null(context_t *ctx, lir_t *lir) {
    return operand_imm(ctx->reports, value_int(lir->node, lir_type(lir), 0));
}

static operand_t emit_cast(context_t *ctx, lir_t *lir) {
    operand_t it = emit_lir(ctx, lir->src);
    step_t step = step_of(ctx, OP_CAST, lir);
    step.src = it;
    return add_step(ctx, step);
}

static operand_t compile_offset(context_t *ctx, lir_t *lir, operand_t base, operand_t offset) {
    step_t step = step_of(ctx, OP_OFFSET, lir);
    step.src = base;
    step.offset = offset;
    return add_step(ctx, step);
}

static operand_t emit_offset(context_t *ctx, lir_t *lir) {
    operand_t it = emit_lir(ctx, lir->src);
    operand_t offset = emit_lir(ctx, lir->offset);
    return compile_offset(ctx, lir, it, offset);
}

static operand_t emit_local(context_t *ctx, lir_t *lir) {
    if (lir->data != NULL) {
        return operand_address(lir->data);
    }

    block_t *local = local_declare(lir);
    vector_push(&ctx->block->locals, local);
    operand_t dst = operand_address(local);

    if (lir->init) {
        operand_t init = emit_lir(ctx, lir->init);
        compile_store(ctx, lir, dst, init);
    }

    return dst;
}

static operand_t emit_list(context_t *ctx, lir_t *lir) {
    const type_t *type = lir_type(lir);
    vector_t *list = lir->elements;
    size_t len = vector_len(list);
    where_t where = lir->node->where;
    char *name = format("array%zu%zu", where.first_line, where.first_column);

    block_t *block = detailed_block(name, type, &DEFAULT_ATTRIBS, lir->node);
    vector_push(&ctx->block->locals, block);
    operand_t dst = operand_address(block);

    for (size_t i = 0; i < len; i++) {
        lir_t *element = vector_get(list, i);
        operand_t src = emit_lir(ctx, element);
        operand_t idx = operand_imm(ctx->reports, value_int(element->node, type_usize(), (long)i));
        operand_t offset = compile_offset(ctx, lir, dst, idx);
        compile_store(ctx, element, offset, src);
    }

    return dst;
}

static operand_t emit_lir(context_t *ctx, lir_t *lir) {
    switch (lir->leaf) {
    case LIR_UNARY: return emit_unary(ctx, lir);
    case LIR_BINARY: return emit_binary(ctx, lir);
    case LIR_DIGIT: return emit_digit(ctx, lir);
    case LIR_BOOL: return emit_bool(ctx, lir);
    case LIR_NULL: return emit_null(ctx, lir);
    case LIR_STRING: return emit_string(ctx, lir);
    case LIR_STMTS: return emit_stmts(ctx, lir);
    case LIR_ASSIGN: return emit_assign(ctx, lir);
    case LIR_WHILE: return emit_while(ctx, lir);
    case LIR_BRANCH: return emit_branch(ctx, lir);
    case LIR_READ: return emit_read(ctx, lir);
    case LIR_CALL: return emit_call(ctx, lir);
    case LIR_VALUE: return emit_value(lir);
    case LIR_LOCAL: return emit_local(ctx, lir);
    case LIR_DEFINE: return emit_define(lir);
    case LIR_PARAM: return emit_param(lir);
    case LIR_RETURN: return emit_return(ctx, lir);
    case LIR_POISON: return emit_poison(ctx, lir);
    case LIR_FORWARD: return emit_forward(ctx, lir);
    case LIR_BREAK: return emit_break(ctx, lir);
    case LIR_CAST: return emit_cast(ctx, lir);
    case LIR_OFFSET: return emit_offset(ctx, lir);
    case LIR_LIST: return emit_list(ctx, lir);

    case LIR_DETAIL_SIZEOF: return emit_sizeof(ctx, lir);
    case LIR_DETAIL_ALIGNOF: return emit_alignof(ctx, lir);

    default:
        ctu_assert(ctx->reports, "emit-lir unknown %d", lir->leaf);
        return operand_empty();
    }
}

static module_t *init_module(vector_t *vars, vector_t *funcs, const char *name) {
    module_t *mod = ctu_malloc(sizeof(module_t));
    mod->name = name;
    mod->vars = vars;
    mod->funcs = funcs;
    return mod;
}

static vector_t *collect_vars(vector_t *vec) {
    vector_t *all = vector_new(64);
    size_t len = vector_len(vec);
    for (size_t i = 0; i < len; i++) {
        lir_t *lir = vector_get(vec, i);
        vector_push(&all, lir->vars);
    }
    return vector_collect(all);
}

static vector_t *collect_funcs(vector_t *vec) {
    vector_t *all = vector_new(64);
    size_t len = vector_len(vec);
    for (size_t i = 0; i < len; i++) {
        lir_t *lir = vector_get(vec, i);
        vector_push(&all, lir->funcs);
    }
    return vector_collect(all);
}

static vector_t *collect_imports(vector_t *vec) {
    vector_t *all = vector_new(64);
    size_t len = vector_len(vec);
    for (size_t i = 0; i < len; i++) {
        lir_t *lir = vector_get(vec, i);
        vector_push(&all, lir->imports);
    }
    return vector_collect(all);
}

static char *module_name(const char *base) {
    vector_t *parts = strsplit(base, PATH_SEP);
    return strjoin("::", parts);
}

module_t *module_build(reports_t *reports, const char *base, vector_t *nodes) {
    vector_t *vars = collect_vars(nodes); // root->vars;
    size_t nvars = vector_len(vars);

    vector_t *funcs = collect_funcs(nodes); // root->funcs;
    size_t nfuncs = vector_len(funcs);

    vector_t *varblocks = vector_of(nvars);
    vector_t *funcblocks = vector_of(nfuncs);
    vector_t *types = vector_new(0);

    vector_t *strings = vector_new(4);

    module_t *mod = init_module(varblocks, funcblocks, module_name(base));

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

    vector_t *imports = collect_imports(nodes);
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
        context_t ctx = build_block(strings, reports, mod, block, var->init);
        strings = ctx.strings;
        types = vector_join(types, ctx.types);
    }

    for (size_t i = 0; i < nfuncs; i++) {
        lir_t *func = vector_get(funcs, i);
        block_t *block = vector_get(funcblocks, i);
        context_t ctx = build_define(strings, reports, mod, block, func);
        strings = ctx.strings;
        types = vector_join(types, ctx.types);
    }

    mod->imports = symbols;
    mod->strtab = strings;
    mod->types = types;

    return mod;
}