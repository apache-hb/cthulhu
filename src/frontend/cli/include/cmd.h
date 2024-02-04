#pragma once

#include <stdbool.h>

#include "setup/setup.h"

typedef struct logger_t logger_t;
typedef struct map_t map_t;
typedef struct ap_t ap_t;
typedef struct vector_t vector_t;

typedef struct tool_t
{
    cfg_group_t *config;

    cfg_field_t *emit_ssa;
    cfg_field_t *output_dir;
    cfg_field_t *output_header;
    cfg_field_t *output_source;
    cfg_field_t *output_reflect;

    cfg_field_t *warn_as_error;
    cfg_field_t *report_limit;
    cfg_field_t *report_style;

    default_options_t options;
} tool_t;

tool_t make_tool(arena_t *arena);
