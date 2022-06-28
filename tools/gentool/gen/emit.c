#include "emit.h"
#include "gen.h"
#include "util.h"

#include <string.h>

#include "std/str.h"
#include "std/map.h"

#include "base/macros.h"

#include "report/report.h"
#include "report/report-ext.h"

#include "cJSON.h"

#define CONFIG_NAME "name" /// the user facing name of the language
#define CONFIG_VERSION "version" /// the version of this language
#define CONFIG_CHANGELOG "changelog" /// relative path to the changelog file
#define CONFIG_ID "id" /// the internal name of this language
#define CONFIG_DESC "desc" /// short description of the language
#define CONFIG_EXTS "exts" /// possible file extensions for this language

static const char *get_string(reports_t *reports, map_t *map, const char *field)
{
    const ast_t *value = map_get(map, field);
    
    if (value == NULL) 
    {
        report(reports, ERROR, node_invalid(), "missing field '%s'", field);
        return "";
    }

    if (value->kind != AST_STRING && value->kind != AST_IDENT)
    {
        report(reports, ERROR, value->node, "expected string");
        return "";
    }

    return value->str;
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

static bool emit_include(emit_t *emit, file_t file, const char *path, const char *pre)
{
    return write_to(emit, file, "%s#include \"%s\"\n", pre, path);
}

#define CHECK(expr) do { if (!(expr)) { return; } } while (0)
#define WRITE(fn, ...) CHECK(fn(config, file, __VA_ARGS__))
#define WRITE_LINE() WRITE(write_to, "\n")

static void emit_header(emit_t *config, file_t file)
{
    WRITE(write_to, "#pragma once\n");
    
    WRITE_LINE();
    WRITE(emit_include, "scan/node.h", "");

    WRITE_LINE();
    WRITE(write_to, "#define %sLTYPE where_t\n", config->upperId);
}

static void emit_source(emit_t *config, file_t file)
{
    WRITE(emit_include, "ast.gen.h", "");
    WRITE(emit_include, "base/macros.h", "");
    WRITE(emit_include, "report/report.h", "");

    WRITE_LINE();
    WRITE(write_to, "void %serror(where_t *where, void *state, scan_t scan, const char *msg)\n", config->id);
    WRITE(write_to, "{\n");
    WRITE(write_to, "\tUNUSED(state);\n");
    WRITE(write_to, "\treport(scan_reports(scan), ERROR, node_new(scan, *where), \"%%s\", msg);\n");
    WRITE(write_to, "}\n");
}


static bool write_attr(emit_t *config, file_t file, const char *cfg, const char *body)
{
    return write_to(config, file, "%%%s %s\n", cfg, body);
}

static bool write_option(emit_t *config, file_t file, const char *options)
{
    return write_attr(config, file, "option", options);
}

static void emit_flex(emit_t *config, file_t file)
{
    WRITE(write_option, "extra-type=\"scan_t\"");
    WRITE(write_option, "8bit nodefault");
    WRITE(write_option, "noyywrap noinput nounput");
    WRITE(write_option, "noyyalloc noyyrealloc noyyfree");
    WRITE(write_option, "reentrant bison-bridge bison-locations");
    WRITE(write_option, "never-interactive batch");
    WRITE(write_option, format("prefix=\"%s\"", config->id));

    WRITE_LINE();
    WRITE(write_to, "%%{\n");
    WRITE(emit_include, "gen-bison.h", "");
    WRITE(emit_include, "report/report-ext.h", "");
    WRITE(emit_include, "interop/flex.h", "");
    WRITE(write_to, "%%}\n");

    WRITE_LINE();
    WRITE(write_to, "%%%%\n");

    WRITE(write_to, ". { report_unknown_character(scan_reports(yyextra), node_new(yyextra, *yylloc), yytext); }\n");
    WRITE(write_to, "%%%%\n");

    WRITE_LINE();
    WRITE(write_to, "FLEX_MEMORY(%salloc, %srealloc, %sfree)\n", config->id, config->id, config->id);
}

static void emit_bison(emit_t *config, file_t file)
{
    WRITE(write_attr, "define", "parse.error verbose");
    WRITE(write_attr, "define", "api.pure full");
    WRITE(write_attr, "lex-param", "{ void *scan }");
    WRITE(write_attr, "parse-param", "{ void *scan } { scan_t x }");
    WRITE(write_attr, "locations", "");
    WRITE(write_attr, "expect", "0");
    WRITE(write_attr, "define", format("api.prefix {%s}", config->id));

    WRITE_LINE();
    WRITE(write_attr, "code", "top {");
    WRITE(emit_include, "interop/flex.h", "\t");
    WRITE(write_to, "}\n");

    WRITE_LINE();
    WRITE(write_attr, "code", "requires {");
    WRITE(emit_include, "ast.gen.h", "\t");
    WRITE(write_to, "\t#define YYLTYPE %sLTYPE\n", config->upperId);
    WRITE(write_to, "\t#define YYSTYLE %sSTYPE\n", config->upperId);
    WRITE(write_to, "}\n");

    WRITE_LINE();
    WRITE(write_to, "%%{\n");
    WRITE(write_to, "int %slex();\n", config->id);
    WRITE(write_to, "void %serror(where_t *where, void *state, scan_t scan, const char *msg);\n", config->id);
    WRITE(write_to, "%%}\n");

    WRITE_LINE();
    WRITE(write_to, "%%%%\n");
    WRITE(write_to, "%%%%\n");
}

#define KEY(x) "\"" x "\""
#define IS ": "
#define STRING(x) "\"" x "\""
#define ARRAY(x) "[" x "]"
#define OBJECT(x) "{" x "}"
#define NL ",\n"

static const char *kPackageJson = 
    "{\n"
    "\t" KEY("name") IS STRING("${id}") NL
    "\t" KEY("displayName") IS STRING("${name}") NL
    "\t" KEY("description") IS STRING("${desc}") NL
    "\t" KEY("version") IS STRING("${version}") NL
    "\t" KEY("engines") IS OBJECT(KEY("vscode") IS STRING("^1.68.0")) NL
    "\t" KEY("categories") IS ARRAY(STRING("Programming Languages")) "\n"
    "}\n"
;

// generate package.json
static void emit_vscode_package(emit_t *config, file_t file)
{
    map_t *cfg = config->root->config->fields;
    const char *name = get_string(config->reports, cfg, CONFIG_NAME);
    const char *desc = get_string(config->reports, cfg, CONFIG_DESC);
    const char *ver = get_string(config->reports, cfg, CONFIG_VERSION);

    map_t *replace = map_optimal(32);
    map_set(replace, "${id}", (char*)config->id);
    map_set(replace, "${name}", (char*)name);
    map_set(replace, "${version}", (char*)ver);
    map_set(replace, "${desc}", (char*)desc);

    const char *result = str_replace_many(kPackageJson, replace);

    WRITE(write_to, "%s\n", result);
}

// generate language-configuration.json
static void emit_vscode_language(emit_t *config, file_t file)
{
    UNUSED(config);
    UNUSED(file);
}

// generate id.tmlang.json
static void emit_vscode_textmate(emit_t *config, file_t file)
{
    UNUSED(config);
    UNUSED(file);
}

static void emit_vscode(emit_t *config)
{
    const char *root = format("%s" NATIVE_PATH_SEPARATOR "%s", config->path, config->id);
    cerror_t err = make_directory(root);
    if (err != 0) 
    {
        report_errno(config->reports, "failed to make vscode directory", err);
        return;
    }

    file_t package = check_open(config->reports, format("%s" NATIVE_PATH_SEPARATOR "package.json", root), FILE_WRITE | FILE_TEXT);
    file_t language = check_open(config->reports, format("%s" NATIVE_PATH_SEPARATOR "language-configuration.json", root), FILE_WRITE | FILE_TEXT);
    file_t tmlang = check_open(config->reports, format("%s" NATIVE_PATH_SEPARATOR "%s.tmlang.json", root, config->id), FILE_WRITE | FILE_TEXT);

    if (file_valid(package))
    {
        emit_vscode_package(config, package);
        file_close(package);
    }

    if (file_valid(language))
    {
        emit_vscode_language(config, language);
        file_close(language);
    }

    if (file_valid(tmlang))
    {
        emit_vscode_textmate(config, tmlang);
        file_close(tmlang);
    }
}

void emit(emit_t *config)
{
    config->id = get_string(config->reports, config->root->config->fields, CONFIG_ID);
    config->upperId = str_upper(config->id);

    file_t header = check_open(config->reports, format("%s" NATIVE_PATH_SEPARATOR "ast.gen.h", config->path), FILE_WRITE | FILE_TEXT);
    file_t source = check_open(config->reports, format("%s" NATIVE_PATH_SEPARATOR "ast.gen.c", config->path), FILE_WRITE | FILE_TEXT);

    file_t flex = check_open(config->reports, format("%s" NATIVE_PATH_SEPARATOR "ast.l", config->path), FILE_WRITE | FILE_TEXT);
    file_t bison = check_open(config->reports, format("%s" NATIVE_PATH_SEPARATOR "ast.y", config->path), FILE_WRITE | FILE_TEXT);

#define EMIT_FILE(file, fn) do { \
    if (file_valid(file)) { \
        fn(config, file); \
        write_to(config, file, "\n"); \
        file_close(file); \
        } \
    } while (0)

    EMIT_FILE(header, emit_header);
    EMIT_FILE(source, emit_source);
    EMIT_FILE(flex, emit_flex);
    EMIT_FILE(bison, emit_bison);
    
    if (config->enableVsCode) 
    {
        emit_vscode(config);
    }
}
