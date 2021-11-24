#include "sema.h"
#include "data.h"
#include "type.h"
#include "value.h"
#include "define.h"
#include "import.h"

#include "ctu/driver/include.h"
#include "ctu/util/util.h"
#include "ctu/util/report-ext.h"

static void add_imports(sema_t *sema, vector_t *imports) {
    size_t len = vector_len(imports);
    for (size_t i = 0; i < len; i++) {
        ctu_t *node = vector_get(imports, i);
        compile_import(sema, node);
    }
}

static leaf_t decl_leaf(ctu_t *ctu) {
    switch (ctu->type) {
    case CTU_VALUE: return LIR_VALUE;
    case CTU_DEFINE: return LIR_DEFINE;

    default: return LIR_FORWARD;
    }
}

lir_t *ctu_forward(node_t *node, const char *name, leaf_t leaf, void *data) {
    return lir_forward(node, is_discard(name) ? NULL : name, leaf, data);
}

static void add_decls(sema_t *sema, vector_t *decls) {
    size_t len = vector_len(decls);
    for (size_t i = 0; i < len; i++) {
        ctu_t *decl = vector_get(decls, i);
        leaf_t leaf = decl_leaf(decl);

        const char *name = decl->name;
     
        switch (decl->type) {
        case CTU_NEWTYPE:
            forward_type(sema, name, decl);
            continue;
        default: break;
        }

        lir_t *lir = ctu_forward(decl->node, name, leaf, state_new(sema, decl));

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
    map_t *types = sema_tag(sema, TAG_USERTYPES);
    MAP_APPLY(types, sema, build_type);

    map_t *vars = sema_tag(sema, TAG_GLOBALS);
    MAP_APPLY(vars, sema, build_value);

    map_t *funcs = sema_tag(sema, TAG_FUNCS);
    MAP_APPLY(funcs, sema, build_define);

    vector_t *defs = MAP_COLLECT(funcs, nonull);
    vector_t *lambdas = move_lambdas(sema);

    lir_t *mod = lir_module(root, 
        /* externs = */ move_externs(sema),
        /* vars = */ map_values(vars),
        /* funcs = */ vector_join(defs, lambdas)
    );

    make_complete(sema, mod);
    return mod;
}

lir_t *ctu_sema(reports_t *reports, ctu_t *ctu) {
    sema_t *sema = ctu_start(reports, ctu);
    lir_t *mod = ctu_finish(sema);
    return mod;
}

sema_t *ctu_start(reports_t *reports, ctu_t *ctu) {
    vector_t *decls = ctu->decls;
    vector_t *imports = ctu->imports;
    const char *path = ctu->node->scan->path;
    sema_t *sema = base_sema(reports, path, ctu, vector_len(decls), vector_len(imports));
    vector_t *header = strsplit(ctu_noext(path), PATH_SEP);

    /* TODO: find a better way of doing module names */
    set_path(sema, header);

    add_imports(sema, imports);
    add_decls(sema, decls);

    return sema;
}

lir_t *ctu_finish(sema_t *sema) {
    if (is_complete(sema)) {
        return cached_lir(sema);
    }

    return compile_decls(sema, get_tree(sema)->node);
}

vector_t *ctu_analyze(reports_t *reports, ctu_t *ctu) {
    ctu_sema(reports, ctu);
    vector_t *cache = cached_data();
    for (size_t i = 0; i < vector_len(cache); i++) {
        sema_t *sema = vector_get(cache, i);
        vector_set(cache, i, ctu_finish(sema));
    }
    return cache;
}
