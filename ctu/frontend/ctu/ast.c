#include "ast.h"

static map_t *ctu_types(void) {
    map_t *types = map_new(8);
    map_set(types, "int", type_digit(true, TY_INT));
    return types;
}

static options_t *ctu_options(void) {
    options_t *opts = ctu_malloc(sizeof(options_t));
    opts->case_sensitive = true;
    opts->order_independent = true;
    opts->types = ctu_types();
    return opts;
}

node_t *ctu_module(scan_t *scan, where_t where, vector_t *decls) {
    return ast_module(scan, where, ctu_options(), decls);
}
