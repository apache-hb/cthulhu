#include "lir.h"

#include "ctu/util/report.h"

static lir_t *lir_new(node_t *node, const type_t *type, leaf_t leaf) {
    lir_t *lir = ctu_malloc(sizeof(lir_t));
    lir->node = node;
    lir->leaf = leaf;
    lir->_type = type;
    lir->data = NULL;
    return lir;
}

const attrib_t DEFAULT_ATTRIBS = {
    .visibility = PRIVATE,
    .mangle = NULL,
    .section = NULL
};

static lir_t *lir_decl(node_t *node, leaf_t leaf, const char *name, const type_t *type) {
    lir_t *lir = lir_new(node, type, leaf);
    lir->_name = name;
    lir->attribs = &DEFAULT_ATTRIBS;
    return lir;
}

void lir_attribs(lir_t *dst, const attrib_t *attribs) {
    dst->attribs = attribs;
}

lir_t *lir_forward(node_t *node, const char *name, leaf_t expected, void *ctx) {
    lir_t *lir = lir_decl(node, LIR_FORWARD, name, type_poison("forward declaration"));
    lir->expected = expected;
    lir->ctx = ctx;
    return lir;
}

lir_t *lir_module(node_t *node, 
                  vector_t *imports, 
                  vector_t *vars, 
                  vector_t *funcs) {
    lir_t *lir = lir_new(node, NULL, LIR_MODULE);

    lir->imports = imports;
    lir->vars = vars;
    lir->funcs = funcs;

    return lir;
}

lir_t *lir_int(node_t *node, const type_t *type, int digit) {
    lir_t *lir = lir_new(node, type, LIR_DIGIT);

    mpz_init_set_si(lir->digit, digit);

    return lir;
}

lir_t *lir_digit(node_t *node, const type_t *type, mpz_t digit) {
    lir_t *lir = lir_new(node, type, LIR_DIGIT);

    mpz_init_set(lir->digit, digit);

    return lir;
}

lir_t *lir_string(node_t *node, const type_t *type, const char *str) {
    lir_t *lir = lir_new(node, type, LIR_STRING);

    lir->str = str;

    return lir;
}

lir_t *lir_bool(node_t *node, const type_t *type, bool value) {
    lir_t *lir = lir_new(node, type, LIR_BOOL);

    lir->boolean = value;

    return lir;
}

lir_t *lir_null(node_t *node, const type_t *type) {
    return lir_new(node, type, LIR_NULL);
}

lir_t *lir_list(node_t *node, const type_t *type, vector_t *elements) {
    lir_t *lir = lir_new(node, type, LIR_LIST);

    lir->elements = elements;

    return lir;
}

lir_t *lir_read(node_t *node, const type_t *type, lir_t *src) {
    lir_t *lir = lir_new(node, type, LIR_READ);

    lir->src = src;
    lir->offset = NULL;

    return lir;
}

lir_t *lir_binary(node_t *node, const type_t *type, binary_t binary, lir_t *lhs, lir_t *rhs) {
    lir_t *lir = lir_new(node, type, LIR_BINARY);

    lir->binary = binary;
    lir->lhs = lhs;
    lir->rhs = rhs;

    return lir;
}

lir_t *lir_offset(node_t *node, const type_t *type, lir_t *src, lir_t *offset) {
    lir_t *lir = lir_new(node, type, LIR_OFFSET);

    lir->src = src;
    lir->offset = offset;

    return lir;
}

lir_t *lir_unary(node_t *node, const type_t *type, unary_t unary, lir_t *operand) {
    lir_t *lir = lir_new(node, type, LIR_UNARY);

    lir->unary = unary;
    lir->operand = operand;

    return lir;
}

lir_t *lir_call(node_t *node, const type_t *type, lir_t *func, vector_t *args) {
    lir_t *lir = lir_new(node, type, LIR_CALL);

    lir->func = func;
    lir->args = args;

    return lir;
}

lir_t *lir_cast(node_t *node, const type_t *type, lir_t *expr) {
    lir_t *lir = lir_new(node, type, LIR_CAST);

    lir->src = expr;

    return lir;
}

lir_t *lir_detail_sizeof(node_t *node, const type_t *type) {
    lir_t *lir = lir_new(node, type_usize(), LIR_DETAIL_SIZEOF);

    lir->of = type;

    return lir;
}

lir_t *lir_detail_alignof(node_t *node, const type_t *type) {
    lir_t *lir = lir_new(node, type_usize(), LIR_DETAIL_ALIGNOF);

    lir->of = type;

    return lir;
}

lir_t *lir_assign(node_t *node, lir_t *dst, lir_t *src) {
    lir_t *lir = lir_new(node, NULL, LIR_ASSIGN);

    lir->dst = dst;
    lir->src = src;

    return lir;
}

lir_t *lir_while(node_t *node, lir_t *cond, lir_t *then) {
    lir_t *lir = lir_new(node, NULL, LIR_WHILE);

    lir->cond = cond;
    lir->then = then;

    return lir;
}

lir_t *lir_return(node_t *node, lir_t *operand) {
    lir_t *lir = lir_new(node, NULL, LIR_RETURN);
    lir->operand = operand;
    return lir;
}

lir_t *lir_stmts(node_t *node, vector_t *stmts) {
    lir_t *lir = lir_new(node, NULL, LIR_STMTS);

    lir->stmts = stmts;

    return lir;
}

lir_t *lir_branch(node_t *node, lir_t *cond, lir_t *then, lir_t *other) {
    lir_t *lir = lir_new(node, NULL, LIR_BRANCH);

    lir->cond = cond;
    lir->then = then;
    lir->other = other;

    return lir;
}

lir_t *lir_break(node_t *node, lir_t *loop) {
    lir_t *lir = lir_new(node, NULL, LIR_BREAK);

    lir->loop = loop;

    return lir;
}

lir_t *lir_local(
    node_t *node, 
    const char *name, 
    const type_t *type, 
    lir_t *init
)
{
    lir_t *lir = lir_decl(node, LIR_LOCAL, name, type);
    lir->init = init;
    return lir;
}

void lir_value(reports_t *reports, lir_t *dst, const type_t *type, lir_t *init) {
    if (dst->leaf != LIR_FORWARD) {
        ctu_assert(reports, "lir-value already resolved");
    }

    dst->leaf = LIR_VALUE;
    dst->_type = type;
    dst->init = init;
}

void lir_define(reports_t *reports, lir_t *dst, const type_t *type, lir_t *body) {
    if (dst->leaf != LIR_FORWARD) {
        ctu_assert(reports, "lir-define already resolved");
    }

    dst->leaf = LIR_DEFINE;
    dst->_type = type;
    dst->body = body;   
}

lir_t *lir_param(node_t *node, const char *name, const type_t *type, size_t index) {
    lir_t *lir = lir_decl(node, LIR_PARAM, name, type);
    lir->index = index;
    return lir;
}

lir_t *lir_poison(node_t *node, const char *msg) {
    return lir_new(node, type_poison_with_node(msg, node), LIR_POISON);
}

bool lir_ok(const lir_t *lir) {
    return !lir_is(lir, LIR_POISON);
}

bool lir_is(const lir_t *lir, leaf_t leaf) {
    return lir->leaf == leaf;
}

vector_t *lir_recurses(lir_t *lir, const lir_t *root) {
    vector_t *result = NULL;
    lir_t *source = NULL;

    if (lir == root) {
        return vector_init(lir);
    }

    switch (lir->leaf) {
    case LIR_READ:
        result = lir_recurses(lir->src, root);
        source = lir->src;
        break;

    case LIR_OFFSET:
        if ((result = lir_recurses(lir->src, root))) {
            source = lir->src;
            break;
        }

        if ((result = lir_recurses(lir->offset, root))) {
            source = lir->offset;
            break;
        }
        break;

    case LIR_VALUE:
        result = lir_recurses(lir->init, root);
        source = lir->init;
        break;

    case LIR_BINARY:
        if ((result = lir_recurses(lir->lhs, root))) {
            source = lir->lhs;
            break;
        }
        
        if ((result = lir_recurses(lir->rhs, root))) {
            source = lir->rhs;
            break;
        }
        break;

    case LIR_UNARY:
        result = lir_recurses(lir->operand, root);
        source = lir->operand;
        break;

    default:
        return NULL;
    }

    if (result) {
        vector_push(&result, source);
    }

    return result;
}

const type_t *lir_type(const lir_t *lir) {
    return lir->_type;
}

void retype_lir(lir_t *lir, const type_t *type) {
    lir->_type = type;
}

const char *get_name(const lir_t *lir) {
    return lir->_name;
}

bool has_name(const lir_t *lir) {
    return get_name(lir) != NULL;
}
