#include "sema.h"

typedef struct {
    map_t *types;
    map_t *decls;
} ctu_data_t;

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

static void compile_value(sema_t *sema, lir_t *lir) {
    (void) sema;
    (void) lir;
    assert2(sema->reports, "value not implemented");
}

static lir_t *build_decl(sema_t *sema, lir_t *lir) {
    if (lir->leaf != LIR_FORWARD) {
        return lir;
    }

    if (sema == NULL) {
        sema = lir->ctx;
    }

    switch (lir->leaf) {
    case LIR_VALUE:
        compile_value(sema, lir);
        break;

    default:
        assert2(sema->reports, "unknown leaf");
        break;
    }

    return lir;
}

static void compile_decl(sema_t *sema, lir_t *lir) {
    if (lir->leaf != LIR_FORWARD) {
        return;
    }
    build_decl(sema, lir);
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
