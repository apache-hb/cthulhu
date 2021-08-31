#include "ast.h"

node_t *ctu_module(scan_t *scan, where_t where, vector_t *decls) {
    return ast_module(scan, where, decls);
}
