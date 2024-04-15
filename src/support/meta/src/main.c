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
#include "core/version_def.h"
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

static const cfg_arg_t kSourceOutputArgs[] = { CT_ARG_LONG("source"), CT_ARG_SHORT("s") };

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

    cfg_field_t *header = config_string(root, &kHeaderOutputInfo, "out.h");
    cfg_field_t *source = config_string(root, &kSourceOutputInfo, "out.c");

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
    setup_default(NULL);
    tool_t tool = make_tool();

    setup_init_t init = setup_parse(argc, argv, tool.options);
    if (setup_should_exit(&init))
        return setup_exit_code(&init);
}
