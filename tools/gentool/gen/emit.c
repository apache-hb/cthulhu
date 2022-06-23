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

static bool emit_include(emit_t *emit, file_t file, const char *path)
{
    return write_to(emit, file, "#include \"%s\"\n", path);
}

#define CHECK(expr) do { if (!(expr)) { return; } } while (0)
#define WRITE(fn, ...) CHECK(fn(config, file, __VA_ARGS__))

static void emit_header(emit_t *config, file_t file)
{
    WRITE(write_to, "#pragma once\n");
    WRITE(write_to, "\n");
    WRITE(emit_include, "scan/node.h");
    WRITE(write_to, "\n");
    WRITE(write_to, "#define %sLTYPE where_t\n", config->upperId);
}

static void emit_source(emit_t *config, file_t file)
{
    WRITE(emit_include, "ast.gen.h");
    WRITE(emit_include, "base/macros.h");
    WRITE(emit_include, "report/report.h");
    WRITE(write_to, "\n");
    WRITE(write_to, "void %serror(where_t *where, void *state, scan_t scan, const char *msg)\n", config->id);
    WRITE(write_to, "{\n");
    WRITE(write_to, "\tUNUSED(state);\n");
    WRITE(write_to, "\treport(scan_reports(scan), ERROR, node_new(scan, *where), \"%%s\", msg);\n");
    WRITE(write_to, "}\n");
}

// generate package.json
static void emit_vscode_package(emit_t *config, file_t file)
{
    
}

// generate language-configuration.json
static void emit_vscode_language(emit_t *config, file_t file)
{
    
}

// generate id.tmlang.json
static void emit_vscode_textmate(emit_t *config, file_t file)
{
    
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

    if (file_valid(header)) 
    {
        emit_header(config, header);
        file_close(header);
    }
    
    if (file_valid(source)) 
    {
        emit_source(config, source);
        file_close(source);
    }

    if (config->enableVsCode) 
    {
        emit_vscode(config);
    }
}
