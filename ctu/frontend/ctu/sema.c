#include "sema.h"

#if 0
typedef struct {
    map_t *types;
    map_t *decls;
} ctu_data_t;

typedef struct {
    sema_t *sema;
    ctu_t *ctu;
} ctx_t;

ctx_t *ctx_new(sema_t *sema, ctu_t *ctu) {
    ctx_t *ctx = ctu_malloc(sizeof(ctx_t));
    ctx->sema = sema;
    ctx->ctu = ctu;
    return ctx;
}

static void *ctu_data_new(void) {
    ctu_data_t *data = ctu_malloc(sizeof(ctu_data_t));
    data->types = map_new(4);
    data->decls = map_new(4);
    return data;
}

static void ctu_data_delete(void *data) {
    ctu_data_t *sema = data;
    map_delete(sema->types);
    map_delete(sema->decls);
    ctu_free(sema);
}

#define NEW_SEMA(parent, reports) sema_new(parent, reports, ctu_data_new)
#define DELETE_SEMA(sema) sema_delete(sema, ctu_data_delete)

#define GET_TYPE(sema, name) sema_get(sema, name, ctu_get_type)
#define GET_DECL(sema, name) sema_get(sema, name, ctu_get_decl)

#define SET_TYPE(sema, name, type) sema_set(sema, name, type, ctu_add_type)
#define SET_DECL(sema, name, decl) sema_set(sema, name, decl, ctu_add_decl)

#define CTU_GET(field, func) \
    static lir_t *func(sema_t *sema, const char *name) { \
        ctu_data_t *data = sema->fields; \
        return map_get(data->field, name); \
    }

#define CTU_ADD(field, func) \
    static void func(sema_t *sema, const char *name, lir_t *lir) { \
        ctu_data_t *data = sema->fields; \
        lir_t *other1 = GET_TYPE(sema, name); \
        if (other1 != NULL) { \
            report_shadow(sema->reports, name, lir->node, other1->node); \
        } \
        lir_t *other2 = GET_DECL(sema, name); \
        if (other2 != NULL) { \
            report_shadow(sema->reports, name, lir->node, other2->node); \
        } \
        map_set(data->field, name, lir); \
    } 

static void report_shadow(reports_t *reports, const char *name, node_t *other, node_t *self) {
    message_t *id = report2(reports, ERROR, self, "refinition of `%s`", name);
    report_append2(id, other, "previous definition");
    report_note2(id, "PL/0 is case insensitive");
}

CTU_GET(types, ctu_get_type)
CTU_GET(decls, ctu_get_decl)

//CTU_ADD(types, ctu_add_type)
CTU_ADD(decls, ctu_add_decl)

static leaf_t ctu_leaf(ctu_type_t type) {
    switch (type) {
    case CTU_VALUE: return LIR_VALUE;
    default: return ~0; /* bad */
    }
}

static lir_t *ctu_declare(sema_t *sema, ctu_t *ctu, const char *name, leaf_t leaf) {
    return lir_forward(ctu->node, name, leaf, sema);
}

static void add_global(sema_t *sema, leaf_t leaf, const char *name, ctu_t *ctu) {
    lir_t *lir = ctu_declare(sema, ctu, name, leaf);

    switch (leaf) {
    case LIR_DEFINE: case LIR_VALUE:
        SET_DECL(sema, name, lir);
        break;
    default:
        assert2(sema->reports, "unknown leaf");
        break;
    }
}

static lir_t *compile_digit(ctu_t *digit) {
    lir_t *lir = lir_digit(digit->node, digit->digit);
    lir->type = type_digit(true, TY_INT); /* TODO: type suffixes */
    return lir;
}

static lir_t *compile_unary(sema_t *sema, ctu_t *unary) {
    lir_t *operand = compile_expr(sema, unary->operand);

    lir_t *lir = lir_unary(unary->node, unary->unary, operand);

    return lir;
}

static lir_t *compile_binary(sema_t *sema, ctu_t *binary) {
    lir_t *lhs = compile_unary(sema, binary->lhs);
    lir_t *rhs = compile_unary(sema, binary->rhs);

    lir_t *lir = lir_binary(binary->node, binary->binary, lhs, rhs);

    return lir;
}

static lir_t *compile_expr(sema_t *sema, ctu_t *expr) {
    lir_t *result = NULL;

    switch (expr->type) {
    case CTU_DIGIT:
        result = compile_digit(expr);
        break;
    case CTU_UNARY:
        result = compile_unary(sema, expr);
        break;
    case CTU_BINARY:
        result = compile_binary(sema, expr);
        break;
        
    default:
        assert2(sema->reports, "unknown type");
        result = lir_poison(expr->node, "unknown type");
    }

    return result;
}

static void compile_value(sema_t *sema, lir_t *lir, ctu_t *ctu) {
    lir_t *value = compile_expr(sema, lir->value);
    
    lir_value(sema->reports, lir, value->type, value);
}

static lir_t *build_decl(sema_t *sema, lir_t *lir, ctu_t *ctu) {
    if (lir->leaf != LIR_FORWARD) {
        return lir;
    }

    if (sema == NULL) {
        sema = lir->ctx;
    }

    switch (lir->leaf) {
    case LIR_VALUE:
        compile_value(sema, lir, ctu);
        break;

    default:
        assert2(sema->reports, "unknown leaf");
        break;
    }

    return lir;
}

static void compile_decl(sema_t *sema, ctx_t *ctx) {
    lir_t *lir = ctx->lir;
    if (lir->leaf != LIR_FORWARD) {
        return;
    }
    build_decl(sema, lir, ctx->ctu);
}

#define IS_LEAF(func, it) \
    static bool func(lir_t *lir) { \
        return lir->leaf == it; \
    }

IS_LEAF(is_value, LIR_VALUE)
IS_LEAF(is_define, LIR_DEFINE)

lir_t *ctu_sema(reports_t *reports, ctu_t *ctu) {
    sema_t *sema = NEW_SEMA(NULL, reports);

    vector_t *decls = ctu->decls;
    size_t len = vector_len(decls);

    for (size_t i = 0; i < len; i++) {
        ctu_t *decl = vector_get(decls, i);
        leaf_t leaf = ctu_leaf(decl->type);
        const char *name = decl->name;
        add_global(sema, leaf, name, decl);
    }

    ctu_data_t *data = sema->fields;

    MAP_APPLY(data->decls, sema, compile_decl);

    vector_t *values = MAP_COLLECT(data->decls, is_value);
    vector_t *defines = MAP_COLLECT(data->decls, is_define);

    DELETE_SEMA(sema);

    return lir_module(ctu->node, values, defines);
}
#endif

typedef enum {
    TAG_TYPES,
    TAG_VARS,
    TAG_FUNCS,

    TAG_MAX
} ctu_tag_t;

#define NEW_SEMA(parent, reports, sizes) \
    sema_new(parent, reports, TAG_MAX, sizes)

#define DELETE_SEMA(sema) \
    sema_delete(sema)

lir_t *ctu_sema(reports_t *reports, ctu_t *ctu) {
    UNUSED(ctu);

    sema_t *sema = NEW_SEMA(NULL, reports, NULL);

    DELETE_SEMA(sema);

    return NULL;
}
