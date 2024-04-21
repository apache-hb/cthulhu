// SPDX-License-Identifier: LGPL-3.0-only

/// meta command line tool to generate repetitive data
/// output types:
/// * diagnostic and error message listings for a component
///   - in structured format (json/xml/html) for generating docs
///   - in C for use inside the component
/// * command line flags for a component
///   - both structured and C output
/// * ast node definitions for a language
///   - in C for use in the compiler
///   - maybe generate pretty printers or metadata for the ast
///   - some sort of serialized format for fuzzing and object files
///     - maybe protobuf for fuzzing
///     - a custom format for object files?
/// * lexer and parser definitions for a language
///   - flex/bison output for use in the compiler
///   - vim/textmate/emacs syntax highlighting definitions

#include "config/config.h"
#include "core/macros.h"
#include "core/version_def.h"
#include "format/config.h"
#include "format/notify2.h"
#include "io/console.h"
#include "io/io.h"
#include "memory/memory.h"
#include "meta.h"
#include "notify/notify.h"
#include "os/os.h"
#include "setup/setup.h"
#include "std/vector.h"
#include "json/json.h"

static const version_info_t kToolVersion = {
    .license = "LGPLv3",
    .author = "Elliot Haisley",
    .desc = "Meta code generation tool",
    .version = CT_NEW_VERSION(0, 0, 1),
};

typedef struct tool_t
{
    cfg_group_t *root;

    cfg_field_t *header_output;
    cfg_field_t *source_output;

    setup_options_t options;
} tool_t;

static const cfg_info_t kRootInfo = {
    .name = "meta",
    .brief = "meta code generation tool",
};

static const cfg_arg_t kHeaderOutputArgs[] = { CT_ARG_LONG("header") };

static const cfg_info_t kHeaderOutputInfo = {
    .name = "header",
    .brief = "output header file",
    .args = CT_ARGS(kHeaderOutputArgs),
};

static const cfg_arg_t kSourceOutputArgs[] = { CT_ARG_LONG("source") };

static const cfg_info_t kSourceOutputInfo = {
    .name = "source",
    .brief = "output source file",
    .args = CT_ARGS(kSourceOutputArgs),
};

static tool_t make_tool(void)
{
    arena_t *arena = get_global_arena();

    cfg_group_t *root = config_root(&kRootInfo, arena);
    setup_options_t options = setup_options(kToolVersion, root);

    cfg_field_t *header = config_string(root, &kHeaderOutputInfo, NULL);
    cfg_field_t *source = config_string(root, &kSourceOutputInfo, NULL);

    tool_t tool = {
        .root = root,
        .header_output = header,
        .source_output = source,
        .options = options,
    };

    return tool;
}

static void print_errors(io_t *io, setup_init_t setup, logger_t *logger, arena_t *arena)
{
    print_notify_t config = {
        .options = {
            .arena = arena,
            .io = io,
            .pallete = setup.pallete,
        },
        .heading = setup.heading,
        .style = eNotifyFull,
        .zero_indexed_lines = false,
    };

    print_notify_many(config, logger_get_events(logger));
}

int main(int argc, const char **argv)
{
    setup_default(NULL);
    arena_t *arena = get_global_arena();
    tool_t tool = make_tool();

    setup_init_t init = setup_parse(argc, argv, tool.options);
    if (setup_should_exit(&init))
        return setup_exit_code(&init);

    size_t len = vector_len(init.posargs);
    if (len != 1)
        return setup_exit_help(tool.options, &init);

    const char *input = vector_get(init.posargs, 0);

    io_t *err = io_stderr();
    io_t *con = io_stdout();

    io_t *io = io_file(input, eOsAccessRead, arena);
    io_error_t error = io_error(io);
    if (error != eOsSuccess)
    {
        io_printf(err, "failed to open file %s: %s\n", input, os_error_string(error, arena));
        return CT_EXIT_ERROR;
    }

    logger_t *logger = logger_new(arena);

    json_parse_t parse = json_parse(io, logger, arena);
    if (parse.root == NULL)
    {
        print_errors(err, init, logger, arena);
        return CT_EXIT_ERROR;
    }

    meta_info_t *info = meta_info_parse(parse.root, parse.scanner, logger, arena);
    if (info == NULL)
    {
        print_errors(err, init, logger, arena);
        return CT_EXIT_ERROR;
    }

    text_view_t prefix = info->prefix;
    io_printf(con, "prefix: %.*s\n", (int)prefix.length, prefix.text);

    size_t nodes = typevec_len(info->nodes);
    for (size_t i = 0; i < nodes; i++)
    {
        meta_ast_t *ast = typevec_offset(info->nodes, i);
        text_view_t ast_name = ast->name;
        io_printf(con, "node: %.*s\n", (int)ast_name.length, ast_name.text);

        size_t fields = typevec_len(ast->fields);
        for (size_t j = 0; j < fields; j++)
        {
            meta_field_t *field = typevec_offset(ast->fields, j);
            text_view_t field_name = field->name;
            io_printf(con, "field: %.*s\n", (int)field_name.length, field_name.text);
        }
    }
}
