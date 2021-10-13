#include "sema.h"
#include "data.h"
#include "type.h"
#include "value.h"
#include "define.h"

#include "ctu/util/util.h"
#include "ctu/util/report-ext.h"

static leaf_t decl_leaf(ctu_t *ctu) {
    switch (ctu->type) {
    case CTU_VALUE: return LIR_VALUE;
    case CTU_DEFINE: return LIR_DEFINE;

    default: return LIR_FORWARD;
    }
}

static void add_decls(sema_t *sema, vector_t *decls) {
    size_t len = vector_len(decls);
    for (size_t i = 0; i < len; i++) {
        ctu_t *decl = vector_get(decls, i);
        leaf_t leaf = decl_leaf(decl);

        const char *name = decl->name;
        lir_t *lir = lir_forward(decl->node, name, leaf, state_new(sema, decl));

        switch (decl->type) {
        case CTU_VALUE:
            add_var(sema, name, lir);
            break;
        case CTU_DEFINE:
            add_func(sema, name, lir);
            break;
        default:
            ctu_assert(sema->reports, "add-decls unreachable %d", decl->type);
            break;
        }
    }
}

static bool nonull(lir_t *lir) {
    return lir->body != NULL;
}

static lir_t *compile_decls(sema_t *sema, node_t *root) {
    map_t *vars = sema_tag(sema, TAG_GLOBALS);
    MAP_APPLY(vars, sema, build_value);

    map_t *funcs = sema_tag(sema, TAG_FUNCS);
    MAP_APPLY(funcs, sema, build_define);

    return lir_module(root, 
        /* externs = */ move_externs(sema),
        /* vars = */ map_values(vars),
        /* funcs = */ MAP_COLLECT(funcs, nonull)
    );
}

lir_t *ctu_sema(reports_t *reports, ctu_t *ctu) {
    vector_t *decls = ctu->decls;
    size_t ndecls = vector_len(decls);
    sema_t *sema = base_sema(reports, ndecls);

    add_decls(sema, decls);
    lir_t *mod = compile_decls(sema, ctu->node);

    sema_delete(sema);
    return mod;
}
