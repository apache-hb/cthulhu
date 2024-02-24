// SPDX-License-Identifier: LGPL-3.0-only

#include "core/version_def.h"
#include "config/config.h"

#include "setup/setup.h"

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
