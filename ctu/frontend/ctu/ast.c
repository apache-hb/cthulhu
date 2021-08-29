#include "ast.h"

static const options_t OPTIONS = {
    .case_sensitive = true,
    .order_independent = true
};

node_t *ctu_module(scan_t *scan, where_t where, vector_t *decls) {
    return ast_module(scan, where, &OPTIONS, decls);
}
