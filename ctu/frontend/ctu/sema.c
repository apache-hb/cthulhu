#include "sema.h"
#include "ctu/util/report-ext.h"

typedef enum {
    TAG_TYPES,
    TAG_VARS,
    TAG_FUNCS,

    TAG_MAX
} ctu_tag_t;

typedef struct {
    ctu_t *decl;
    sema_t *sema;
} ctx_t;

static ctx_t *ctx_new(ctu_t *decl, sema_t *sema) {
    ctx_t *ctx = ctu_malloc(sizeof(ctx_t));
    ctx->decl = decl;
    ctx->sema = sema;
    return ctx;
}

#define NEW_SEMA(parent, reports, sizes) \
    sema_new(parent, reports, TAG_MAX, sizes)

#define DELETE_SEMA(sema) \
    sema_delete(sema)

static leaf_t ctu_leaf(ctu_t *decl) {
    switch (decl->type) {
    case CTU_VALUE: return LIR_VALUE;
    default: return LIR_POISON;
    }
}

static ctu_tag_t ctu_tag(ctu_t *decl) {
    switch (decl->type) {
    case CTU_VALUE: return TAG_VARS;
    default: return TAG_VARS;
    }
}

static lir_t *ctu_declare(sema_t *sema, ctu_t *decl) {
    leaf_t leaf = ctu_leaf(decl);
    return lir_forward(decl->node, decl->name, leaf, ctx_new(decl, sema));
}

static void add_global(sema_t *sema, ctu_tag_t tag, const char *name, lir_t *lir) {
    lir_t *var = sema_get(sema, TAG_VARS, name);
    if (var != NULL) {
        report_shadow(sema->reports, name, var->node, lir->node);
    }

    lir_t *func = sema_get(sema, TAG_FUNCS, name);
    if (func != NULL) {
        report_shadow(sema->reports, name, func->node, lir->node);
    }

    sema_set(sema, tag, name, lir);
}

static void compile_type(sema_t *sema, lir_t *decl) {
    UNUSED(sema);
    UNUSED(decl);
}

static void compile_value(sema_t *sema, lir_t *decl) {
    UNUSED(sema);
    UNUSED(decl);
}

static void compile_func(sema_t *sema, lir_t *decl) {
    size_t sizes[TAG_MAX] = {
        [TAG_TYPES] = MAP_SMALL,
        [TAG_VARS] = MAP_SMALL,
        [TAG_FUNCS] = MAP_SMALL
    };
    sema_t *nest = NEW_SEMA(sema, sema->reports, sizes);

    ctx_t *ctx = decl->ctx;

    DELETE_SEMA(nest);
}

lir_t *ctu_sema(reports_t *reports, ctu_t *ctu) {
    vector_t *decls = ctu->decls;
    size_t ndecls = vector_len(decls);

    size_t sizes[TAG_MAX] = {
        [TAG_TYPES] = ndecls,
        [TAG_VARS] = ndecls,
        [TAG_FUNCS] = ndecls
    };

    sema_t *sema = NEW_SEMA(NULL, reports, sizes);

    for (size_t i = 0; i < ndecls; i++) {
        ctu_t *decl = vector_get(decls, i);
        lir_t *lir = ctu_declare(sema, decl);
        add_global(sema, ctu_tag(decl), decl->name, lir);
    }

    map_t *type_map = sema_tag(sema, TAG_TYPES);
    map_t *global_map = sema_tag(sema, TAG_VARS);
    map_t *func_map = sema_tag(sema, TAG_FUNCS);

    MAP_APPLY(type_map, sema, compile_type);
    MAP_APPLY(global_map, sema, compile_value);
    MAP_APPLY(func_map, sema, compile_func);

    vector_t *tys = map_values(type_map);
    vector_t *vars = map_values(global_map);
    vector_t *funcs = map_values(func_map);

    DELETE_SEMA(sema);

    UNUSED(tys);

    return lir_module(ctu->node,
        /* imports = */ vector_of(0),
        /* types = tys, */
        vars,
        funcs
    );
}
