#pragma once

#include "base/analyze.h"
#include <stddef.h>

typedef struct reports_t reports_t;
typedef struct io_t io_t;
typedef struct scan_t scan_t;

typedef struct text_t {
    size_t size;      ///< the number of bytes in the text
    const char *text; ///< the text itself
} text_t;

NODISCARD CONSTFN
const char *scan_language(IN_NOTNULL const scan_t *scan);

NODISCARD CONSTFN
const char *scan_path(IN_NOTNULL const scan_t *scan);

NODISCARD CONSTFN
void *scan_get(IN_NOTNULL scan_t *scan);

void scan_set(IN_NOTNULL scan_t *scan, void *value);

NODISCARD CONSTFN
const char *scan_text(IN_NOTNULL const scan_t *scan);

NODISCARD CONSTFN
size_t scan_size(IN_NOTNULL const scan_t *scan);

NODISCARD CONSTFN
text_t scan_source(IN_NOTNULL const scan_t *scan);

NODISCARD CONSTFN
reports_t *scan_reports(IN_NOTNULL scan_t *scan);

NODISCARD CONSTFN
io_t *scan_src(IN_NOTNULL scan_t *scan);

NODISCARD CONSTFN
scan_t *scan_invalid(void);

NODISCARD RET_RANGE(0, size)
size_t scan_read(scan_t *scan, OUT_WRITES(size) void *dst, size_t size);

NODISCARD
scan_t *scan_io(reports_t *reports, const char *language, io_t *io);
