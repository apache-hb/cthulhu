#include "meta/ast.h"

#include "io/console.h"
#include "io/io.h"

#include "memory/memory.h"
#include "config/config.h"
#include "setup/setup.h"
#include "json/json.h"

#include "core/macros.h"

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
    cfg_field_t *ast = config_string(meta, &kAstInputInfo, "ast.json");

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

    // const char *source_path = cfg_string_value(tool.source);
    // const char *header_path = cfg_string_value(tool.header);
    const char *ast_path = cfg_string_value(tool.ast);

    io_error_t error = 0;
    // io_t *source = io_file(source_path, eAccessWrite, arena);
    // io_t *header = io_file(header_path, eAccessWrite, arena);
    io_t *ast = io_file(ast_path, eAccessRead, arena);

    // if ((error = io_error(source)))
    // {
    //     io_printf(con, "Error opening source file: %s\n", os_error_string(error, arena));
    //     return CT_EXIT_ERROR;
    // }

    // if ((error = io_error(header)))
    // {
    //     io_printf(con, "Error opening header file: %s\n", os_error_string(error, arena));
    //     return CT_EXIT_ERROR;
    // }

    error = io_error(ast);
    if (error)
    {
        io_printf(con, "Error opening ast file: %s\n", os_error_string(error, arena));
        return CT_EXIT_ERROR;
    }

    json_t *json = json_scan(ast, NULL, arena);
    CT_UNUSED(json);

    return CT_EXIT_OK;
}
