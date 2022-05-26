#include "ast.h"

#include "base/util.h"
#include "base/macros.h"

#include "cthulhu/report/report.h"

static ast_t *ast_new(scan_t *scan, where_t where, ast_kind_t kind)
{
    ast_t *ast = ctu_malloc(sizeof(ast_t));
    ast->node = node_new(scan, where);
    ast->kind = kind;
    return ast;
}

ast_t *ast_grammar(scan_t *scan, where_t where, map_t *config, map_t *lexer)
{
    ast_t *ast = ast_new(scan, where, AST_GRAMMAR);
    ast->config = config;
    ast->lexer = lexer;
    return ast;
}

map_t *build_map(scan_t *scan, where_t where, vector_t *entries)
{
    size_t len = vector_len(entries);
    map_t *map = map_optimal(len);

    for (size_t i = 0; i < len; i++)
    {
        ast_t *entry = vector_get(entries, i);
        CTASSERT(entry->kind == AST_PAIR, "entry must be a pair");

        ast_t *old = map_get(map, entry->key);
        if (old != NULL)
        {
            report(scan->reports, ERROR, node_new(scan, where), "duplicate entry `%s`", entry->key);
        }

        map_set(map, entry->key, entry->value);
    }

    return map;
}

ast_t *ast_vector(scan_t *scan, where_t where, vector_t *vector)
{
    ast_t *ast = ast_new(scan, where, AST_VECTOR);
    ast->vector = vector;
    return ast;
}

ast_t *ast_pair(scan_t *scan, where_t where, char *key, ast_t *value)
{
    ast_t *ast = ast_new(scan, where, AST_PAIR);
    ast->key = key;
    ast->value = value;
    return ast;
}

ast_t *ast_string(scan_t *scan, where_t where, char *string)
{
    ast_t *ast = ast_new(scan, where, AST_STRING);
    ast->string = string;
    return ast;
}

ast_t *ast_ident(scan_t *scan, where_t where, char *ident)
{
    ast_t *ast = ast_new(scan, where, AST_IDENT);
    ast->string = ident;
    return ast;
}
