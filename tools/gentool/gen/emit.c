#include "emit.h"

#include <string.h>

#include "std/str.h"
#include "base/macros.h"

#include "report/report.h"
#include "report/report-ext.h"

#define CONFIG_NAME "name"
#define CONFIG_VERSION "version"
#define CONFIG_ID "id"
#define CONFIG_DESC "desc"
#define CONFIG_EXTS "exts"

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
    UNUSED(file);

    WRITE(write_to, "#pragma once\n");
    WRITE(write_to, "\n");
    WRITE(emit_include, "scan/node.h");
    WRITE(write_to, "\n");
    WRITE(write_to, "#define %sLTYPE where_t\n", config->upperId);
}

static void emit_source(emit_t *config, file_t file)
{
    UNUSED(file);

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

void emit(emit_t *config)
{
    config->id = get_string(config->reports, config->root->config->fields, CONFIG_ID);
    config->upperId = str_upper(config->id);

    emit_header(config, config->header);
    emit_source(config, config->source);

    file_close(config->header);
    file_close(config->source);
}
