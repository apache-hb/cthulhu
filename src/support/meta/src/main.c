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
#include "io/console.h"
#include "memory/memory.h"
#include "setup/setup.h"

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

    default_options_t options;
} tool_t;

static const cfg_info_t kRootInfo = {
    .name = "meta",
    .brief = "meta code generation tool",
};

static const char *const kHeaderOutputLongArgs[] = CT_ARGS("header");
static const char *const kHeaderOutputShortArgs[] = CT_ARGS("h");

static const cfg_info_t kHeaderOutputInfo = {
    .name = "header",
    .brief = "output header file",
    .long_args = kHeaderOutputLongArgs,
    .short_args = kHeaderOutputShortArgs,
};

static const char *const kSourceOutputLongArgs[] = CT_ARGS("source");
static const char *const kSourceOutputShortArgs[] = CT_ARGS("s");

static const cfg_info_t kSourceOutputInfo = {
    .name = "source",
    .brief = "output source file",
    .long_args = kSourceOutputLongArgs,
    .short_args = kSourceOutputShortArgs,
};

static tool_t make_tool(void)
{
    arena_t *arena = get_global_arena();

    cfg_group_t *root = config_root(&kRootInfo, arena);

    cfg_field_t *header = config_string(root, &kHeaderOutputInfo, "out.h");
    cfg_field_t *source = config_string(root, &kSourceOutputInfo, "out.c");

    default_options_t options = get_default_options(root);

    tool_t tool = {
        .root = root,
        .header_output = header,
        .source_output = source,
        .options = options,
    };

    return tool;
}

int main(int argc, const char **argv)
{
    setup_global();
    tool_t tool = make_tool();

    tool_config_t config = {
        .arena = get_global_arena(),
        .io = io_stdout(),

        .group = tool.root,
        .version = kToolVersion,

        .argc = argc,
        .argv = argv,
    };

    int err = parse_commands(tool.options, config);
    if (err == CT_EXIT_SHOULD_EXIT)
    {
        return CT_EXIT_OK;
    }
}
