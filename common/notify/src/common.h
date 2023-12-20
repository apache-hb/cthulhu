#pragma once

#include "core/text.h"
#include "notify/text.h"
#include "notify/notify.h"
#include "notify/colour.h"

#include <stddef.h>

typedef struct map_t map_t;
typedef struct scan_t scan_t;

const char *get_severity_name(severity_t severity);
colour_t get_severity_colour(severity_t severity);

const char *get_scan_name(const node_t *node);

typevec_t *all_segments_in_scan(const typevec_t *segments, const node_t *node);
void segments_sort(typevec_t *segments);

char *fmt_node(file_config_t config, const node_t *node);
char *fmt_coloured(const text_colour_t *colours, colour_t idx, const char *fmt, ...);

size_t get_line_number(file_config_t config, const node_t *node);

bool node_has_line(const node_t *node);
size_t get_offset_line(file_config_t config, size_t line);

/// @brief get the width of a number if it were printed as base10
size_t get_num_width(size_t num);

char *fmt_align(size_t width, const char *fmt, ...);

cache_map_t *cache_map_new(size_t size);
void cache_map_delete(cache_map_t *map);

text_cache_t *cache_emplace_file(cache_map_t *map, const char *path);
text_cache_t *cache_emplace_scan(cache_map_t *map, const scan_t *scan);

text_view_t cache_get_line(text_cache_t *cache, size_t line);
size_t cache_count_lines(text_cache_t *cache);

// extract a line of text, converting non-printable characters to their escape codes
// and highlighting the escaped characters
text_t cache_escape_line(text_cache_t *cache, size_t line, const text_colour_t *colours, size_t column_limit);
