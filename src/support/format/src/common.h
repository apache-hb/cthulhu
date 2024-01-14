#pragma once

#include "format/format.h"
#include "notify/notify.h"

#include "format/notify.h"
#include "format/colour.h"
#include "scan/node.h"

#include <stddef.h>

/// colour defines

typedef struct map_t map_t;
typedef struct scan_t scan_t;

severity_t get_severity(const diagnostic_t *diag, bool override_fatal);

const char *get_severity_name(severity_t severity);
colour_t get_severity_colour(severity_t severity);

typevec_t *all_segments_in_scan(const typevec_t *segments, const node_t *node, arena_t *arena);
void segments_sort(typevec_t *segments);

size_t get_line_number(file_config_t config, const node_t *node);

bool node_has_line(const node_t *node);
size_t get_offset_line(bool zero_indexed_lines, size_t line);

/// @brief get the width of a number if it were printed as base10
size_t get_num_width(size_t num);

char *fmt_left_align(arena_t *arena, size_t width, const char *fmt, ...);
char *fmt_right_align(arena_t *arena, size_t width, const char *fmt, ...);

cache_map_t *cache_map_new(size_t size, arena_t *arena);
void cache_map_delete(cache_map_t *map);

text_cache_t *cache_emplace_file(cache_map_t *map, const char *path);
text_cache_t *cache_emplace_scan(cache_map_t *map, const scan_t *scan);

text_view_t cache_get_line(text_cache_t *cache, size_t line);
size_t cache_count_lines(text_cache_t *cache);

// extract a line of text, converting non-printable characters to their escape codes
// and highlighting the escaped characters
text_t cache_escape_line(text_cache_t *cache, size_t line, const colour_pallete_t *colours, size_t column_limit);

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
char *fmt_node_location(source_config_t config, const node_t *node);
