#include "gen/gen.h"

#include "base/macros.h"
#include "base/util.h"

#include "std/map.h"
#include "std/vector.h"

#include "report/report.h"

void generror(where_t *where, void *state, scan_t scan, const char *msg)
{
    UNUSED(state);

    report(scan_reports(scan), ERROR, node_new(scan, *where), "%s", msg);
}

static ast_t *ast_new(scan_t scan, where_t where, ast_kind_t kind)
{
    ast_t *ast = ctu_malloc(sizeof(ast_t));
    ast->node = node_new(scan, where);
    ast->kind = kind;
    return ast;
}

ast_t *ast_root(scan_t scan, where_t where, ast_t *config, ast_t *tokens)
{
    ast_t *ast = ast_new(scan, where, AST_ROOT);
    ast->config = config;
    ast->tokens = tokens;
    return ast;
}

ast_t *ast_config(scan_t scan, where_t where, map_t *fields)
{
    ast_t *ast = ast_new(scan, where, AST_CONFIG);
    ast->fields = fields;
    return ast;
}

ast_t *ast_tokens(scan_t scan, where_t where, map_t *fields)
{
    ast_t *ast = ast_new(scan, where, AST_TOKENS);
    ast->fields = fields;
    return ast;
}

map_t *collect_map(scan_t scan, vector_t *fields)
{
    size_t len = vector_len(fields);
    map_t *result = map_optimal(len);

    for (size_t i = 0; i < len; i++)
    {
        pair_t *pair = vector_get(fields, i);

        if (map_get(result, pair->key) != NULL)
        {
            report(scan_reports(scan), WARNING, node_invalid(), "duplicate key '%s'", pair->key);
        }

        map_set(result, pair->key, pair->value);
    }

    return result;
}

pair_t *pair_new(const char *key, struct ast_t *ast)
{
    pair_t *pair = ctu_malloc(sizeof(pair_t));
    pair->key = key;
    pair->value = ast;
    return pair;
}

ast_t *ast_string(scan_t scan, where_t where, const char *str)
{
    ast_t *ast = ast_new(scan, where, AST_STRING);
    ast->str = str;
    return ast;
}

ast_t *ast_ident(scan_t scan, where_t where, const char *str)
{
    ast_t *ast = ast_new(scan, where, AST_IDENT);
    ast->str = str;
    return ast;
}

ast_t *ast_digit(scan_t scan, where_t where, mpz_t digit)
{
    ast_t *ast = ast_new(scan, where, AST_DIGIT);
    mpz_init_set(ast->digit, digit);
    return ast;
}

ast_t *ast_regex(scan_t scan, where_t where, const char *re)
{
    ast_t *ast = ast_new(scan, where, AST_REGEX);
    ast->str = re;
    return ast;
}

ast_t *ast_pair(scan_t scan, where_t where, ast_t *lhs, ast_t *rhs)
{
    ast_t *ast = ast_new(scan, where, AST_PAIR);
    ast->lhs = lhs;
    ast->rhs = rhs;
    return ast;
}

ast_t *ast_array(scan_t scan, where_t where, vector_t *elems)
{
    ast_t *ast = ast_new(scan, where, AST_ARRAY);
    ast->vec = elems;
    return ast;
}
