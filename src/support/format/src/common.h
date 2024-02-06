#pragma once

#include "format/format.h"
#include "format/colour.h"

#include "core/where.h"

#include <stddef.h>
#include <stdbool.h>

/// colour defines

size_t get_offset_line(bool zero_indexed_lines, size_t line);

/// @brief get the width of a number if it were printed as base10
size_t get_num_width(size_t num);

char *fmt_left_align(arena_t *arena, size_t width, const char *fmt, ...);
char *fmt_right_align(arena_t *arena, size_t width, const char *fmt, ...);

///
/// version 2 of the common stuff
///

typedef struct source_config_t
{
    format_context_t context;
    colour_t colour;
    heading_style_t heading_style;
    bool zero_indexed_lines;
} source_config_t;

format_context_t format_context_make(print_options_t options);
char *fmt_source_location(source_config_t config, const char *path, where_t where);
