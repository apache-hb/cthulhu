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


/// sparse reports are an efficient and easy to manipulate in-memory representation of a report

typedef struct sparse_text_t sparse_text_t;
typedef struct sparse_report_t sparse_report_t;

/// @brief create a new sparse report
/// this builds the internal data structures for the report
///
/// @param event the event to create the report from
///
/// @return the new sparse report
sparse_report_t *sparse_report_new(const event_t *event);

/// @brief get all the distinct source files in a sparse report
/// the primary files is first, then this is sorted by the file name
/// for the rest of the files.
///
/// @param report the report to get the source files of
///
/// @return the source files in @p report
vector_t *sparse_report_get_files(const sparse_report_t *report);
