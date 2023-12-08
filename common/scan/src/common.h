#pragma once

#include "scan/scan.h"

typedef struct scan_t
{
    reports_t *reports; ///< the reporting sink for this file
    io_t *io;           ///< file itself
    alloc_t *alloc;     ///< allocator to use everything involving this file

    const char *language; ///< the language this file contains
    void *data;           ///< user data pointer

    const char *mapped;
    size_t size;
} scan_t;
