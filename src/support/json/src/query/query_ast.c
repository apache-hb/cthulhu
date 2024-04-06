// SPDX-License-Identifier: LGPL-3.0-or-later
#include "query_ast.h"

#include "arena/arena.h"
#include "base/panic.h"
#include "scan/scan.h"

static query_ast_t *query_ast_new(scan_t *scan, query_ast_type_t type)
{
    CTASSERT(scan != NULL);

    arena_t *arena = scan_get_arena(scan);
    query_ast_t *ast = ARENA_MALLOC(sizeof(query_ast_t), "query", scan, arena);
    ast->kind = type;

    return ast;
}

query_ast_t *query_ast_object(scan_t *scan, text_t name)
{
    query_ast_t *ast = query_ast_new(scan, eQueryObject);
    ast->name = name;

    ARENA_IDENTIFY(ast->name.text, "name", ast, scan_get_arena(scan));

    return ast;
}

query_ast_t *query_ast_field(scan_t *scan, query_ast_t *object, text_t field)
{
    query_ast_t *ast = query_ast_new(scan, eQueryField);
    ast->object = object;
    ast->field = field;

    ARENA_IDENTIFY(ast->object, "object", ast, scan_get_arena(scan));
    ARENA_IDENTIFY(ast->field.text, "field", ast, scan_get_arena(scan));

    return ast;
}

query_ast_t *query_ast_index(scan_t *scan, query_ast_t *object, mpz_t index)
{
    query_ast_t *ast = query_ast_new(scan, eQueryIndex);
    ast->object = object;
    mpz_init_set(ast->index, index);
    return ast;
}

query_ast_t *query_ast_map(scan_t *scan, query_ast_t *object, text_t field)
{
    query_ast_t *ast = query_ast_new(scan, eQueryMap);
    ast->object = object;
    ast->field = field;

    ARENA_IDENTIFY(ast->object, "object", ast, scan_get_arena(scan));
    ARENA_IDENTIFY(ast->field.text, "field", ast, scan_get_arena(scan));

    return ast;
}
