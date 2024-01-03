#include "cpp/ast.h"
#include "base/panic.h"
#include "memory/arena.h"

static cpp_ast_t *cpp_ast_new(const scan_t *scan, where_t where, cpp_kind_t kind)
{
    CTASSERT(scan != NULL);

    arena_t *arena = scan_get_arena(scan);
    const node_t *node = node_new(scan, where);

    cpp_ast_t *ast = ARENA_MALLOC(arena, sizeof(cpp_ast_t), "cpp_ast_t", scan);
    ast->node = node;
    ast->kind = kind;

    return ast;
}

cpp_ast_t *cpp_ast_ident(const scan_t *scan, where_t where, text_t text)
{
    cpp_ast_t *ast = cpp_ast_new(scan, where, eCppIdent);
    ast->ident = text;
    return ast;
}
