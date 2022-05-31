#pragma once

#include <stddef.h>

typedef struct
{
    size_t size;      ///< the number of bytes in the text
    const char *text; ///< the text itself
} text_t;

typedef unsigned scan_t;
typedef struct reports_t reports_t;

const char *scan_language(scan_t scan);
const char *scan_path(scan_t scan);
void *scan_get(scan_t scan);
void scan_set(scan_t scan, void *value);

const char *scan_text(scan_t scan);
size_t scan_size(scan_t scan);
text_t scan_source(scan_t scan);
size_t scan_offset(scan_t scan);
void scan_advance(scan_t scan, size_t offset);

reports_t *scan_reports(scan_t scan);

scan_t scan_invalid(void);
