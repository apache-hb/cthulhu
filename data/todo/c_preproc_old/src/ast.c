#include "cpp/ast.h"
#include "base/panic.h"
#include "memory/arena.h"

static cpp_ast_t *cpp_ast_new(scan_t *scan, where_t where, cpp_kind_t kind)
{
    CTASSERT(scan != NULL);

    cpp_ast_t ast = {
        .kind = kind,
        .node = node_new(scan, where),
    };

    return arena_memdup(&ast, sizeof(cpp_ast_t), scan_get_arena(scan));
}

cpp_ast_t *cpp_define(scan_t *scan, where_t where, const char *name, vector_t *body)
{
    CTASSERT(scan != NULL);
    CTASSERT(name != NULL);
    CTASSERT(body != NULL);

    cpp_ast_t *ast = cpp_ast_new(scan, where, eCppDefine);
    ast->name = name;
    ast->body = body;

    return ast;
}

cpp_ast_t *cpp_macro(scan_t *scan, where_t where, const char *name, cpp_params_t params, vector_t *body)
{
    CTASSERT(name != NULL);
    CTASSERT(body != NULL);

    cpp_ast_t *ast = cpp_ast_new(scan, where, eCppMacro);
    ast->name = name;
    ast->params = params;
    ast->body = body;

    return ast;
}

static cpp_ast_t *ast_anytext(scan_t *scan, where_t where, const char *text, cpp_kind_t kind)
{
    CTASSERT(text != NULL);

    cpp_ast_t *ast = cpp_ast_new(scan, where, kind);
    ast->text = text;

    return ast;
}

cpp_ast_t *cpp_string(scan_t *scan, where_t where, const char *text)
{
    return ast_anytext(scan, where, text, eCppString);
}

cpp_ast_t *cpp_ident(scan_t *scan, where_t where, const char *text)
{
    return ast_anytext(scan, where, text, eCppIdent);
}

cpp_ast_t *cpp_lparen(scan_t *scan, where_t where)
{
    return cpp_ast_new(scan, where, eCppLParen);
}

cpp_ast_t *cpp_rparen(scan_t *scan, where_t where)
{
    return cpp_ast_new(scan, where, eCppRParen);
}

cpp_ast_t *cpp_comma(scan_t *scan, where_t where)
{
    return cpp_ast_new(scan, where, eCppComma);
}

cpp_ast_t *cpp_stringify(scan_t *scan, where_t where, const char *text)
{
    return ast_anytext(scan, where, text, eCppStringify);
}

cpp_ast_t *cpp_concat(scan_t *scan, where_t where)
{
    return cpp_ast_new(scan, where, eCppConcat);
}

cpp_ast_t *cpp_paste(scan_t *scan, where_t where, const char *text)
{
    return ast_anytext(scan, where, text, eCppPaste);
}

cpp_ast_t *cpp_number(scan_t *scan, where_t where, cpp_number_t number)
{
    CTASSERT(number.text != NULL);

    cpp_ast_t *ast = cpp_ast_new(scan, where, eCppNumber);
    ast->original = number.text;
    mpz_init_set_str(ast->digit, number.text, number.base);

    return ast;
}

cpp_ast_t *cpp_number_int(arena_t *arena, const node_t *node, int i)
{
    CTASSERT(node != NULL);

    cpp_ast_t ast = {
        .kind = eCppNumber,
        .node = node,

        .original = "0", // TODO: wrong
    };
    mpz_init_set_si(ast.digit, i);

    return arena_memdup(&ast, sizeof(cpp_ast_t), arena);
}

cpp_ast_t *cpp_binary(scan_t *scan, where_t where, binary_t op)
{
    cpp_ast_t *ast = cpp_ast_new(scan, where, eCppBinary);
    ast->binary = op;

    return ast;
}

cpp_ast_t *cpp_unary(scan_t *scan, where_t where, unary_t op)
{
    cpp_ast_t *ast = cpp_ast_new(scan, where, eCppUnary);
    ast->unary = op;

    return ast;
}

cpp_ast_t *cpp_compare(scan_t *scan, where_t where, compare_t op)
{
    cpp_ast_t *ast = cpp_ast_new(scan, where, eCppCompare);
    ast->compare = op;

    return ast;
}

cpp_ast_t *cpp_ternary(scan_t *scan, where_t where)
{
    return cpp_ast_new(scan, where, eCppTernary);
}

cpp_ast_t *cpp_colon(scan_t *scan, where_t where)
{
    return cpp_ast_new(scan, where, eCppColon);
}

cpp_ast_t *cpp_defined(scan_t *scan, where_t where)
{
    return cpp_ast_new(scan, where, eCppDefined);
}

bool cpp_ast_is(cpp_ast_t *ast, cpp_kind_t kind)
{
    return ast != NULL && ast->kind == kind;
}

bool cpp_ast_is_not(cpp_ast_t *ast, cpp_kind_t kind)
{
    return ast == NULL || ast->kind != kind;
}
