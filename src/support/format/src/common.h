#pragma once

#include "common_simple.h"

#include "core/text.h"
#include "format/notify.h"
#include "notify/notify.h"

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

char *fmt_node_location(source_config_t config, const node_t *node);
