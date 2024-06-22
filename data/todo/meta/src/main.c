#include "format/colour.h"
#include "format/notify.h"
#include "meta/ast.h"

#include "io/console.h"
#include "io/io.h"

#include "memory/memory.h"
#include "config/config.h"
#include "meta/meta.h"
#include "notify/notify.h"
#include "setup/setup.h"
#include "std/vector.h"
#include "interop/compile.h"

#include "core/macros.h"

#include "meta_bison.h" // IWYU pragma: keep
#include "meta_flex.h" // IWYU pragma: keep

CT_CALLBACKS(kCallbacks, meta);

#define NEW_EVENT(name, ...) const diagnostic_t kEvent_##name = __VA_ARGS__;
#include "meta/meta.def"

static const version_info_t kToolVersion = {
    .license = "GPLv3",
    .desc = "Generates compiler source code from metadata",
    .author = "Elliot Haisley",
    .version = CT_NEW_VERSION(0, 1, 0),
};

static const char *const kSourcePathArgs[] = { "source", NULL };
static const cfg_info_t kSourcePathInfo = {
    .name = "source",
    .brief = "The output source file",
    .short_args = kSourcePathArgs,
};

static const char *const kHeaderPathArgs[] = { "header", NULL };
static const cfg_info_t kHeaderPathInfo = {
    .name = "header",
    .brief = "The output header file",
    .short_args = kHeaderPathArgs,
};

static const char *const kAstInputArgs[] = { "ast", NULL };
static const cfg_info_t kAstInputInfo = {
    .name = "ast",
    .brief = "The input AST file",
    .short_args = kAstInputArgs,
};

static const cfg_info_t kMetaGroupInfo = {
    .name = "meta",
    .brief = "Metadata tool options"
};

typedef struct tool_t
{
    default_options_t options;
    cfg_group_t *meta;
    cfg_field_t *source;
    cfg_field_t *header;
    cfg_field_t *ast;
} tool_t;

static tool_t make_tool(arena_t *arena)
{
    cfg_group_t *meta = config_root(&kMetaGroupInfo, arena);
    cfg_field_t *source = config_string(meta, &kSourcePathInfo, "ast.c");
    cfg_field_t *header = config_string(meta, &kHeaderPathInfo, "ast.h");
    cfg_field_t *ast = config_string(meta, &kAstInputInfo, "ast.meta");

    default_options_t options = get_default_options(meta);

    tool_t tool = {
        .options = options,
        .meta = meta,
        .source = source,
        .header = header,
        .ast = ast
    };

    return tool;
}

int main(int argc, const char **argv)
{
    setup_global();
    arena_t *arena = get_global_arena();
    io_t *con = io_stdout();
    tool_t tool = make_tool(arena);
    tool_config_t config = {
        .arena = arena,
        .io = con,
        .group = tool.meta,
        .version = kToolVersion,
        .argc = argc,
        .argv = argv,
    };

    int err = parse_commands(tool.options, config);
    if (err == CT_EXIT_SHOULD_EXIT)
    {
        return CT_EXIT_OK;
    }

    io_error_t error = 0;

    const char *ast_path = cfg_string_value(tool.ast);
    io_t *io = io_file(ast_path, eAccessRead, arena);
    error = io_error(io);
    if (error != 0)
    {
        io_printf(con, "error: failed to open ast file: %s (%s)\n", ast_path, os_error_string(error, arena));
        return CT_EXIT_ERROR;
    }

    logger_t *logger = logger_new(arena);

    meta_scan_t context = {
        .logger = logger,
        .arena = arena
    };

    text_config_t text_config = {
        .colours = &kColourDefault,
        .io = con
    };

    report_config_t report_config = {
        .max_errors = 20,
        .max_warnings = 20,
        .report_format = eTextSimple,
        .text_config = text_config,
    };

    scan_t *scan = scan_io("meta", io, arena);
    scan_set_context(scan, &context);

    parse_result_t result = scan_buffer(scan, &kCallbacks);
    if (result.result != eParseOk)
    {
        text_report(logger_get_events(logger), report_config, "parse error");
        io_printf(con, "error: failed to parse ast file: %s\n", ast_path);
        return CT_EXIT_ERROR;
    }

    meta_ast_t *ast = result.tree;

    const char *src_path = cfg_string_value(tool.source);
    io_t *src = io_file(src_path, eAccessWrite | eAccessTruncate, arena);

    error = io_error(src);
    if (error != 0)
    {
        io_printf(con, "error: failed to open source file: %s (%s)\n", src_path, os_error_string(error, arena));
        return CT_EXIT_ERROR;
    }

    const char *hdr_path = cfg_string_value(tool.header);
    io_t *hdr = io_file(hdr_path, eAccessWrite | eAccessTruncate, arena);

    error = io_error(hdr);
    if (error != 0)
    {
        io_printf(con, "error: failed to open header file: %s (%s)\n", hdr_path, os_error_string(error, arena));
        return CT_EXIT_ERROR;
    }

    meta_emit(ast, hdr, src, arena);

    io_close(src);
    io_close(hdr);

    return CT_EXIT_OK;
}
