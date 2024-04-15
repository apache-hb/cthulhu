// SPDX-License-Identifier: LGPL-3.0-or-later
#include "meta/meta.h"

#include "core/version_def.h"
#include "config/config.h"
#include "memory/memory.h"
#include "setup/setup.h"

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

static const version_info_t kToolVersion = {
    .license = "LGPLv3",
    .author = "Elliot Haisley",
    .desc = "Meta code generation tool",
    .version = CT_NEW_VERSION(0, 0, 1),
};

static const cfg_info_t kRootInfo = {
    .name = "meta",
    .brief = "meta code generation tool",
};

static const cfg_arg_t kModeArgs[] = { CT_ARG_LONG("mode") };

static const cfg_info_t kModeInfo = {
    .name = "mode",
    .brief = "output mode",
    .args = CT_ARGS(kModeArgs),
};

static const cfg_choice_t kModeOptions[] = {

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

static arena_t *gArena;
static setup_options_t gOptions;
static setup_init_t gSetupInfo;

bool meta_init(int argc, const char **argv, const char *name)
{
    setup_default(NULL);

    gArena = get_global_arena();
    cfg_group_t *root = config_root(&kRootInfo, gArena);
    gOptions = setup_options(kToolVersion, root);

    cfg_enum_t mode_info = {

    };

    cfg_field_t *mode = config_enum(root, &kModeInfo, mode_info);
}

int meta_get_exit_code(void)
{

}

void meta_cmdline_arg(const char *name, const char *id, const char *brief, ...)
{

}
