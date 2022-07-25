#pragma once

#include "base/analyze.h"
#include <stddef.h>

typedef struct reports_t reports_t;
typedef struct io_t io_t;
typedef struct scan_t scan_t;
typedef struct alloc_t alloc_t;

typedef struct
{
    size_t size;      ///< the number of bytes in the text
    const char *text; ///< the text itself
} text_t;

typedef struct 
{
    alloc_t *alloc;
    alloc_t *nodeAlloc; ///< allocator for internal nodes
    alloc_t *astAlloc; ///< allocator for driver ast nodes
    alloc_t *yyAlloc; ///< allocator for flex + bison buffers
} scan_config_t;

NODISCARD CONSTFN const char *scan_language(const scan_t *scan);

NODISCARD CONSTFN const char *scan_path(const scan_t *scan);

NODISCARD CONSTFN void *scan_get(scan_t *scan);

void scan_set(scan_t *scan, void *value);

NODISCARD CONSTFN const char *scan_text(const scan_t *scan);

NODISCARD CONSTFN size_t scan_size(const scan_t *scan);

NODISCARD CONSTFN text_t scan_source(const scan_t *scan);

NODISCARD CONSTFN reports_t *scan_reports(scan_t *scan);

NODISCARD CONSTFN scan_t *scan_invalid(void);

NODISCARD RET_RANGE(0, size)
size_t scan_read(scan_t *scan, void *dst, size_t size);

NODISCARD
scan_t *scan_io(reports_t *reports, const char *language, io_t *io, scan_config_t config);

NODISCARD
void *ast_alloc(scan_t *scan, size_t size, const char *name);

alloc_t *scan_yyalloc(scan_t *scan);