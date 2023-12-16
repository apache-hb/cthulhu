#pragma once

#include "notify/notify.h"
#include "notify/format.h"

typedef struct map_t map_t;

const char *get_severity_name(severity_t severity);
colour_t get_severity_colour(severity_t severity);

const char *get_scan_name(const node_t *node);

typevec_t *all_segments_in_scan(const typevec_t *segments, const node_t *node);
void segments_sort(typevec_t *segments);

char *fmt_node(file_config_t config, const node_t *node);
char *fmt_coloured(text_colour_t colours, colour_t idx, const char *fmt, ...);

size_t get_line_number(file_config_t config, const node_t *node);
