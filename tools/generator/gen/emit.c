#include "emit.h"

#include "std/str.h"
#include "base/util.h"
#include "base/macros.h"

static const char *kTypeNames[AST_TOTAL] = {
    [AST_GRAMMAR] = "grammar",
    [AST_MAP] = "map",
    [AST_VECTOR] = "sequence",
    [AST_STRING] = "string",
    [AST_PAIR] = "pair",
};

static const char *expect_string(reports_t *reports, ast_t *ast)
{
    if (ast->kind != AST_STRING)
    {
        report(reports, ERROR, ast->node, "expected string, got %s instead", kTypeNames[ast->kind]);
        return "";
    }

    return ast->string;
}

static vector_t *empty_vector_of(node_t node, size_t size)
{
    vector_t *vector = vector_of(size);
    ast_t *id = ast_ident(get_node_scanner(node), get_node_location(node), ctu_strdup("empty"));

    for (size_t i = 0; i < size; i++)
    {
        vector_set(vector, i, id);
    }

    return vector;
}

static vector_t *expect_vector(reports_t *reports, ast_t *ast, size_t size)
{
    if (ast->kind != AST_VECTOR)
    {
        report(reports, ERROR, ast->node, "expected sequence, got %s instead", kTypeNames[ast->kind]);
        return empty_vector_of(ast->node, size);
    }

    vector_t *vec = ast->vector;
    size_t len = vector_len(vec);

    if (size == SIZE_MAX)
    {
        return vec;
    }

    if (len != size)
    {
        report(reports, ERROR, ast->node, "expected sequence of length %zu, got %zu instead", size, len);
        return empty_vector_of(ast->node, size);
    }

    return vec;
}

static const char *get_string(reports_t *reports, map_t *map, const char *key, bool optional)
{
    ast_t *ast = map_get(map, key);
    if (ast == NULL)
    {
        if (optional)
        {
            return NULL;
        }

        report(reports, ERROR, node_invalid(), "missing required key `%s`", key);
        return "";
    }

    return expect_string(reports, ast);
}

static vector_t *get_vector(reports_t *reports, map_t *map, const char *key, bool optional)
{
    ast_t *ast = map_get(map, key);
    if (ast == NULL)
    {
        if (optional)
        {
            return NULL;
        }

        report(reports, ERROR, node_invalid(), "missing required key `%s`", key);
        return vector_new(0);
    }

    if (ast->kind != AST_VECTOR)
    {
        report(reports, ERROR, ast->node, "expected sequence `%s`", key);
        return vector_new(0);
    }

    return ast->vector;
}

static const char *kLexerOptions = 
    "%option extra-type=\"scan_t*\"\n"
    "%option yylineno 8bit nodefault\n"
    "%option noyywrap noinput nounput\n"
    "%option noyyalloc noyyrealloc noyyfree\n"
    "%option reentrant bison-bridge bison-locations\n"
    "%option never-interactive batch\n"
    "%option prefix=\"%s\"\n"
;

static void write_include(stream_t *stream, const char *path)
{
    stream_write(stream, format("#include \"%s\"\n", path));
}

static void write_rules(reports_t *reports, stream_t *stream, vector_t *rules)
{
    if (rules == NULL)
    {
        return;
    }

    for (size_t i = 0; i < vector_len(rules); i++)
    {
        ast_t *ast = vector_get(rules, i);
        vector_t *rule = expect_vector(reports, ast, SIZE_MAX);
        size_t len = vector_len(rule);

        if (len < 2)
        {
            report(reports, ERROR, ast->node, "rule must provide at least a pattern and name");
            continue;
        }

        const char *pattern = expect_string(reports, vector_get(rule, 0));
        const char *name = expect_string(reports, vector_get(rule, 1));
        const char *exec = len > 2 ? expect_string(reports, vector_get(rule, 2)) : "";

        stream_write(stream, format("%s { %s return %s; }\n", pattern, exec, name));
    }
}

static stream_t *emit_lexer(reports_t *reports, ast_t *ast)
{
    stream_t *out = stream_new(0x1000);

    const char *prefix = get_string(reports, ast->config, "prefix", false);
    const char *whitespace = get_string(reports, ast->config, "whitespace", false);

    // header includes
    vector_t *includes = get_vector(reports, ast->config, "includes", true);
    
    // comments
    vector_t *lineComments = get_vector(reports, ast->config, "line-comments", true);
    vector_t *blockComments = get_vector(reports, ast->config, "block-comments", true);

    // actual symbols
    vector_t *keywords = get_vector(reports, ast->config, "keywords", false);
    vector_t *tokens = get_vector(reports, ast->config, "symbols", false);
    vector_t *digits = get_vector(reports, ast->config, "digits", false);
    vector_t *strings = get_vector(reports, ast->config, "strings", false);
    vector_t *idents = get_vector(reports, ast->config, "idents", false);

    stream_write(out, format(kLexerOptions, prefix));

    stream_write(out, "%{\n");

    write_include(out, format("%s-bison.h", prefix));

    if (includes != NULL)
    {
        for (size_t i = 0; i < vector_len(includes); i++)
        {
            ast_t *include = vector_get(includes, i);
            write_include(out, expect_string(reports, include));
        }
    }

    stream_write(out, "%}\n");

    if (blockComments != NULL)
    {
        stream_write(out, "%x COMMENT\n"); // TODO: generate state per block comment type
    }

    stream_write(out, "%%\n");

    stream_write(out, format("%s ;", whitespace));

    if (lineComments != NULL)
    {
        for (size_t i = 0; i < vector_len(lineComments); i++)
        {
            ast_t *comment = vector_get(lineComments, i);
            stream_write(out, format("%s.* ;", expect_string(reports, comment)));
        }
    }

    if (blockComments != NULL)
    {
        for (size_t i = 0; i < vector_len(blockComments); i++)
        {
            ast_t *comment = vector_get(blockComments, i);
            vector_t *parts = expect_vector(reports, comment, 2);
            
            const char *start = expect_string(reports, vector_get(parts, 0));
            const char *end = expect_string(reports, vector_get(parts, 1));

            stream_write(out, format("%s { BEGIN(COMMENT); }\n", start));
            stream_write(out, format("<COMMENT>%s { BEGIN(INITIAL); }\n", end));

            stream_write(out, "<COMMENT>\\n ;\n");
            stream_write(out, "<COMMENT>. ;\n");
        }
    }

    write_rules(reports, out, keywords);
    write_rules(reports, out, tokens);
    write_rules(reports, out, digits);
    write_rules(reports, out, strings);
    write_rules(reports, out, idents);

    stream_write(out, ". { report_unknown_character(yyextra->reports, node_new(yyextra, *yylloc), yytext); }\n");

    stream_write(out, "%%\n");

    char *memory = format("FLEX_MEMORY(%salloc, %srealloc, %sfree)\n", prefix, prefix, prefix);
    stream_write(out, memory);

    return out;
}

map_t *emit_compiler(reports_t *reports, ast_t *ast)
{
    map_t *map = map_new(0x10);

    map_set(map, "out.l", emit_lexer(reports, ast));

    return map;
}

map_t *emit_tooling(reports_t *reports, ast_t *ast)
{
    map_t *map = map_new(0x10);

    UNUSED(reports);
    UNUSED(ast);

    return map;
}
