#pragma once

#include "base/analyze.h"
#include <stddef.h>

typedef struct
{
    size_t size;      ///< the number of bytes in the text
    const char *text; ///< the text itself
} text_t;

typedef unsigned scan_t;
typedef struct reports_t reports_t;

NODISCARD CONSTFN const char *scan_language(scan_t scan);

NODISCARD CONSTFN const char *scan_path(scan_t scan);

NODISCARD CONSTFN void *scan_get(scan_t scan);

void scan_set(scan_t scan, void *value);

NODISCARD CONSTFN const char *scan_text(scan_t scan);

NODISCARD CONSTFN size_t scan_size(scan_t scan);

NODISCARD CONSTFN text_t scan_source(scan_t scan);

NODISCARD CONSTFN size_t scan_offset(scan_t scan);

void scan_advance(scan_t scan, size_t offset);

NODISCARD CONSTFN reports_t *scan_reports(scan_t scan);

NODISCARD CONSTFN scan_t scan_invalid(void);
