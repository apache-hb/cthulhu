#include "c/ast.h"
#include "base/panic.h"
#include "memory/arena.h"

static c_ast_t *c_ast_new(scan_t *scan, where_t where, c_kind_t kind)
{
    CTASSERT(scan != NULL);
    arena_t *arena = scan_get_arena(scan);
    node_t *node = node_new(scan, where);

    c_ast_t *ast = ARENA_MALLOC(arena, sizeof(c_ast_t), "c_ast_t", scan);
    ast->node = node;
    ast->kind = kind;
    return ast;
}

c_ast_t *c_ast_label(scan_t *scan, where_t where, const char *name)
{
    CTASSERT(scan != NULL);
    CTASSERT(name != NULL);

    c_ast_t *ast = c_ast_new(scan, where, eAstLabel);
    ast->label = name;
    return ast;
}

c_ast_t *c_ast_case(scan_t *scan, where_t where, c_ast_t *value)
{
    CTASSERT(scan != NULL);
    CTASSERT(value != NULL);

    c_ast_t *ast = c_ast_new(scan, where, eAstCase);
    ast->case_value = value;
    return ast;
}

c_ast_t *c_ast_default(scan_t *scan, where_t where)
{
    CTASSERT(scan != NULL);

    c_ast_t *ast = c_ast_new(scan, where, eAstDefault);
    return ast;
}
