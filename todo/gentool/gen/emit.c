#include "emit.h"
#include "gen.h"
#include "util.h"

#include <string.h>

#include "std/map.h"
#include "std/str.h"
#include "std/vector.h"

#include "base/macros.h"

#include "report/report-ext.h"
#include "report/report.h"

#include "cJSON.h"

#define CONFIG_NAME "name"           /// the user facing name of the language
#define CONFIG_VERSION "version"     /// the version of this language
#define CONFIG_CHANGELOG "changelog" /// relative path to the changelog file
#define CONFIG_ID "id"               /// the internal name of this language
#define CONFIG_DESC "desc"           /// short description of the language
#define CONFIG_EXTS "exts"           /// possible file extensions for this language

#define TOKENS_IGNORE "ignore"
#define TOKENS_LINE_COMMENT "line-comment"
#define TOKENS_BLOCK_COMMENT "block-comment"

static bool is_string(const ast_kind_t kind)
{
    return kind == AST_STRING || kind == AST_IDENT;
}

static const char *get_string(reports_t *reports, map_t *map, const char *field, bool req)
{
    const ast_t *value = map_get(map, field);

    if (value == NULL)
    {
        if (req)
        {
            report(reports, ERROR, node_invalid(), "missing field '%s'", field);
        }
        return "";
    }

    if (!is_string(value->kind))
    {
        report(reports, ERROR, value->node, "expected string");
        return "";
    }

    return value->str;
}

static vector_t *get_array_str(reports_t *reports, map_t *map, const char *field)
{
    const ast_t *value = map_get(map, field);

    if (value == NULL)
    {
        report(reports, ERROR, node_invalid(), "missing field '%s'", field);
        return vector_new(0);
    }

    if (value->kind != AST_ARRAY)
    {
        report(reports, ERROR, value->node, "expected array");
        return vector_new(0);
    }

    vector_t *vec = value->vec;
    size_t len = vector_len(vec);
    vector_t *result = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        const ast_t *at = vector_get(vec, i);
        if (!is_string(at->kind))
        {
            report(reports, ERROR, at->node, "expected array of strings");
            return vector_new(0);
        }

        vector_set(result, i, format("\"%s\"", at->str));
    }

    return result;
}

typedef struct
{
    const char *lhs;
    const char *rhs;
} both_t;

static both_t get_pair_str(reports_t *reports, map_t *map, const char *field, bool req)
{
    const ast_t *value = map_get(map, field);

    both_t result = {};

    if (value == NULL)
    {
        if (req)
        {
            report(reports, ERROR, node_invalid(), "missing field '%s'", field);
        }

        return result;
    }

    if (value->kind != AST_PAIR)
    {
        report(reports, ERROR, value->node, "expected pair");
        return result;
    }

    ast_t *lhs = value->lhs;
    ast_t *rhs = value->rhs;

    if (!is_string(lhs->kind))
    {
        report(reports, ERROR, lhs->node, "lhs was not a string");
        return result;
    }

    if (!is_string(rhs->kind))
    {
        report(reports, ERROR, rhs->node, "rhs was not a string");
        return result;
    }

    result.lhs = lhs->str;
    result.rhs = rhs->str;

    return result;
}

static bool write_to(emit_t *emit, file_t file, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    const char *str = formatv(fmt, args);

    va_end(args);

    cerror_t err = 0;
    file_write(file, str, strlen(str), &err);

    if (err != 0)
    {
        report_errno(emit->reports, "failed to write to file", err);
        return false;
    }

    return true;
}

#define CHECK(expr)                                                                                                    \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(expr))                                                                                                   \
        {                                                                                                              \
            return;                                                                                                    \
        }                                                                                                              \
    } while (0)
#define WRITE(fn, ...) CHECK(fn(config, file, __VA_ARGS__))
#define WRITE_LINE() WRITE(write_to, "\n")

#define KEY(x) "\"" x "\""
#define IS ": "
#define STRING(x) "\"" x "\""
#define ARRAY(x) "[" x "]"
#define OBJECT(x) "{" x "}"
#define NL ",\n"

#define INCLUDE(x) "#include " STRING(x) "\n"
#define DEFINE(k, v) "#define " k " " v "\n"
#define OPTION(a) "%option " a "\n"
#define SET(k, v) "%define " k " " v "\n"

// clang-format off
static const char *kHeaderFile = 
    "#pragma once\n"
    "\n"
    INCLUDE("scan/node.h")
    "\n"
    DEFINE("${upper-id}LTYPE", "where_t")
;

static const char *kSourceFile = 
    INCLUDE("ast.gen.h")
    INCLUDE("base/macros.h")
    INCLUDE("report/report.h")
    "\n"
    "void ${id}error(where_t *where, void *state, scan_t scan, const char *msg)\n"
    "{\n"
    "\tUNUSED(state);\n"
    "\treport(scan_reports(scan), ERROR, node_new(scan, *where), \"%s\", msg);\n"
    "}"
;

static const char *kFlexFile = 
    OPTION("extra-type=" STRING("scan_t"))
    OPTION("8bit nodefault")
    OPTION("noyywrap noinput nounput")
    OPTION("noyyalloc noyyrealloc noyyfree")
    OPTION("reentrant bison-bridge bison-locations")
    OPTION("never-interactive batch")
    OPTION("prefix=" STRING("${id}"))
    "\n"
    "%{\n"
    "\t" INCLUDE("gen-bison.h")
    "\t" INCLUDE("report/report-ext.h")
    "\t" INCLUDE("interop/flex.h")
    "%}\n"
    "\n"
    "${states}\n"
    "%%\n"
    "${rules}\n"
    ". { report_unknown_character(scan_reports(yyextra), node_new(yyextra, *yylloc), yytext); }\n"
    "%%\n"
    "\n"
    "FLEX_MEMORY(${id}alloc, ${id}realloc, ${id}free)\n"
;

static const char *kBisonFile = 
    SET("define", "parse.error.verbose")
    SET("define", "api.pure full")
    SET("lex-param", "{ void *scan }")
    SET("parse-param", "{ void *scan } { scan_t x }")
    SET("locations", "")
    SET("expect", "0")
    SET("define", "api.prefix {${id}}")
    "\n"
    "%code top {\n"
    "\t" INCLUDE("interop/flex.h")
    "}\n"
    "\n"
    "%code requires {\n"
    "\t" INCLUDE("ast.gen.h")
    "\t" DEFINE("YYLTYPE", "${upper-id}LTYPE")
    "\t" DEFINE("YYSTYPE", "${upper-id}STYPE")
    "}\n"
    "\n"
    "%{\n"
    "int ${id}lex();\n"
    "void ${id}error(where_t *where, void *state, scan_t scan, const char *msg);\n"
    "%}\n"
    "\n"
    "%%\n"
    "${rules}\n"
    "%%\n"
;

static const char *kPackageJson =
    "{\n"
    "\t" KEY("name") IS STRING("${id}") NL 
    "\t" KEY("displayName") IS STRING("${name}") NL 
    "\t" KEY("description") IS STRING("${desc}") NL 
    "\t" KEY("version") IS STRING("${version}") NL 
    "\t" KEY("engines") IS OBJECT(KEY("vscode") IS STRING("^1.68.0")) NL 
    "\t" KEY("categories") IS ARRAY(STRING("Programming Languages")) NL
    "\t" KEY("contributes") IS OBJECT(
        KEY("languages") IS ARRAY(OBJECT(
            KEY("id") IS STRING("${id}") ","
            KEY("aliases") IS ARRAY(STRING("${name}")) ","
            KEY("extensions") IS ARRAY("${exts}") ","
            KEY("configuration") IS STRING("./language-configuration.json")
        )) ","
        KEY("grammars") IS ARRAY(OBJECT(
            KEY("language") IS STRING("${id}") ","
            KEY("scopeName") IS STRING("source.${id}") ","
            KEY("path") IS STRING("./${id}.tmlang.json")
        )) 
    ) "\n"
    "}"
;

static const char *kLanguageJson = 
    "{\n"
    "\t" KEY("comments") IS OBJECT("${comments}") NL
    "\t" KEY("brackets") IS ARRAY("${brackets}") NL
    "\t" KEY("autoClosingPairs") IS ARRAY("${pairs}") NL
    "\t" KEY("surroundingPairs") IS ARRAY("${pairs}") "\n"
    "}"
;

static const char *kTextMateJson = 
    "{\n"
    "\t" KEY("$schema") IS STRING("https://raw.githubusercontent.com/martinring/tmlanguage/master/tmlanguage.json") NL
    "\t" KEY("name") IS STRING("${name}") NL
    "\t" KEY("patterns") IS ARRAY("") NL
    "\t" KEY("repository") IS OBJECT("${repo}") NL
    "\t" KEY("scopeName") IS STRING("${scope}") "\n"
    "}"
;
// clang-format on


static void emit_header(emit_t *config, file_t file)
{
    map_t *replace = map_optimal(32);
    map_set(replace, "${upper-id}", (char*)config->upperId);

    const char *result = str_replace_many(kHeaderFile, replace);

    WRITE(write_to, "%s", result);
}

static void emit_source(emit_t *config, file_t file)
{
    map_t *replace = map_optimal(32);
    map_set(replace, "${id}", (char*)config->id);

    const char *result = str_replace_many(kSourceFile, replace);

    WRITE(write_to, "%s", result);
}

static void emit_flex(emit_t *config, file_t file)
{
    map_t *toks = config->root->tokens->fields;

    const char *ignore = get_string(config->reports, toks, TOKENS_IGNORE, false);
    const char *lineComment = get_string(config->reports, toks, TOKENS_LINE_COMMENT, false);
    both_t blockComment = get_pair_str(config->reports, toks, TOKENS_BLOCK_COMMENT, false);

    vector_t *rules = vector_new(16);
    vector_push(&rules, format("%s ;", ignore));

    if (strlen(lineComment) > 0)
    {
        vector_push(&rules, format("\"%s\".* ;", lineComment));
    }

    const char *states = "";
    if (blockComment.lhs != NULL && blockComment.rhs != NULL) 
    {
        states = "%x COMMENT";
        vector_push(&rules, format("\"%s\" { BEGIN(COMMENT); }", blockComment.lhs));
        vector_push(&rules, format("<COMMENT>\"%s\" { BEGIN(INITIAL); }", blockComment.rhs));
        vector_push(&rules, "<COMMENT>\\n ;");
        vector_push(&rules, "<COMMENT>. ;");
    }

    map_t *replace = map_optimal(32);
    map_set(replace, "${id}", (char*)config->id);
    map_set(replace, "${states}", (char*)states);
    map_set(replace, "${rules}", str_join("\n", rules));

    const char *result = str_replace_many(kFlexFile, replace);

    WRITE(write_to, "%s", result);
}

static void emit_bison(emit_t *config, file_t file)
{
    map_t *replace = map_optimal(32);
    map_set(replace, "${id}", (char*)config->id);
    map_set(replace, "${upper-id}", (char*)config->upperId);

    const char *result = str_replace_many(kBisonFile, replace);

    WRITE(write_to, "%s", result);
}

// generate package.json
static void emit_vscode_package(emit_t *config, file_t file)
{
    map_t *cfg = config->root->config->fields;
    const char *name = get_string(config->reports, cfg, CONFIG_NAME, true);
    const char *desc = get_string(config->reports, cfg, CONFIG_DESC, false);
    const char *ver = get_string(config->reports, cfg, CONFIG_VERSION, false);
    vector_t *exts = get_array_str(config->reports, cfg, CONFIG_EXTS);

    map_t *replace = map_optimal(32);
    map_set(replace, "${id}", (char *)config->id);
    map_set(replace, "${name}", (char *)name);
    map_set(replace, "${version}", (char *)ver);
    map_set(replace, "${desc}", (char *)desc);
    map_set(replace, "${exts}", str_join(", ", exts));

    const char *result = str_replace_many(kPackageJson, replace);

    WRITE(write_to, "%s", result);
}

// generate language-configuration.json
static void emit_vscode_language(emit_t *config, file_t file)
{
    map_t *toks = config->root->tokens->fields;
    const char *lineComment = get_string(config->reports, toks, TOKENS_LINE_COMMENT, false);
    both_t blockComment = get_pair_str(config->reports, toks, TOKENS_BLOCK_COMMENT, false);

    vector_t *comments = vector_new(2);
    if (strlen(lineComment) > 0)
    {
        vector_push(&comments, format("\"lineComment\": \"%s\"", lineComment));
    }

    if (blockComment.lhs != NULL && blockComment.rhs != NULL)
    {
        vector_push(&comments, format("\"blockComment\": [ \"%s\", \"%s\" ]", blockComment.lhs, blockComment.rhs));
    }

    map_t *replace = map_optimal(32);
    if (vector_len(comments) > 0)
    {
        map_set(replace, "${comments}", str_join(", ", comments));
    }
    else
    {
        map_set(replace, "${comments}", (char*)"");
    }

    const char *result = str_replace_many(kLanguageJson, replace);

    WRITE(write_to, "%s", result);
}

// generate id.tmlang.json
static void emit_vscode_textmate(emit_t *config, file_t file)
{
    map_t *cfg = config->root->config->fields;
    const char *name = get_string(config->reports, cfg, CONFIG_NAME, true);
    
    map_t *replace = map_optimal(32);
    map_set(replace, "${name}", (char *)name);

    const char *result = str_replace_many(kTextMateJson, replace);

    WRITE(write_to, "%s", result);
}

#define EMIT_FILE(file, fn)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        if (file_valid(file))                                                                                          \
        {                                                                                                              \
            fn(config, file);                                                                                          \
            write_to(config, file, "\n");                                                                              \
            file_close(file);                                                                                          \
        }                                                                                                              \
    } while (0)

static void emit_vscode(emit_t *config)
{
    const char *root = format("%s" NATIVE_PATH_SEPARATOR "%s", config->path, config->id);
    cerror_t err = make_directory(root);
    if (err != 0)
    {
        report_errno(config->reports, "failed to make vscode directory", err);
        return;
    }

    char *packagePath = format("%s" NATIVE_PATH_SEPARATOR "package.json", root);
    char *languagePath = format("%s" NATIVE_PATH_SEPARATOR "language-configuration.json", root);
    char *tmlangPath = format("%s" NATIVE_PATH_SEPARATOR "%s.tmlang.json", root, config->id);

    file_t package = check_open(config->reports, packagePath, eFileWrite | eFileText);
    file_t language = check_open(config->reports, languagePath, eFileWrite | eFileText);
    file_t tmlang = check_open(config->reports, tmlangPath, eFileWrite | eFileText);

    EMIT_FILE(package, emit_vscode_package);
    EMIT_FILE(language, emit_vscode_language);
    EMIT_FILE(tmlang, emit_vscode_textmate);
}

void emit(emit_t *config)
{
    config->id = get_string(config->reports, config->root->config->fields, CONFIG_ID, true);
    config->upperId = str_upper(config->id);

    char *headerPath = format("%s" NATIVE_PATH_SEPARATOR "ast.gen.h", config->path);
    char *sourcePath = format("%s" NATIVE_PATH_SEPARATOR "ast.gen.c", config->path);
    char *flexPath = format("%s" NATIVE_PATH_SEPARATOR "ast.l", config->path);
    char *bisonPath = format("%s" NATIVE_PATH_SEPARATOR "ast.y", config->path);

    file_t header = check_open(config->reports, headerPath, eFileWrite | eFileText);
    file_t source = check_open(config->reports, sourcePath, eFileWrite | eFileText);

    file_t flex = check_open(config->reports, flexPath, eFileWrite | eFileText);
    file_t bison = check_open(config->reports, bisonPath, eFileWrite | eFileText);

    EMIT_FILE(header, emit_header);
    EMIT_FILE(source, emit_source);
    EMIT_FILE(flex, emit_flex);
    EMIT_FILE(bison, emit_bison);

    if (config->enableVsCode)
    {
        emit_vscode(config);
    }
}
