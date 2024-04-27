// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "setup/setup.h"

typedef struct logger_t logger_t;
typedef struct support_t support_t;
typedef struct map_t map_t;
typedef struct ap_t ap_t;
typedef struct vector_t vector_t;

typedef struct tool_t
{
    cfg_group_t *config;

    cfg_field_t *add_language;
    cfg_field_t *add_plugin;
    cfg_field_t *add_target;

    cfg_field_t *emit_tree;
    cfg_field_t *emit_ssa;
    cfg_field_t *output_dir;
    cfg_field_t *output_layout;
    cfg_field_t *output_target;

    cfg_field_t *warn_as_error;
    cfg_field_t *report_limit;
    cfg_field_t *report_style;

    setup_options_t options;
} tool_t;

tool_t make_tool(version_info_t version, arena_t *arena);
