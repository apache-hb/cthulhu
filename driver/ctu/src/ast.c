#include "ast.h"

#include "base/memory.h"

static ast_t *ast_new(scan_t *scan, where_t where, ast_kind_t kind)
{
    ast_t *self = ctu_malloc(sizeof(ast_t));
    self->kind = kind;
    self->node = node_new(scan, where);
    return self;
}

ast_t *ast_module(scan_t *scan, where_t where, vector_t *modspec)
{
    ast_t *ast = ast_new(scan, where, eAstModule);
    ast->modspec = modspec;
    return ast;
}
