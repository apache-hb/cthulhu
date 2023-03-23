#pragma once

#include "scan/scan.h"

typedef struct scan_t
{
    reports_t *reports; ///< the reporting sink for this file
    io_t *io;           ///< file itself
    void *extra;        ///< extra data for the scanner

    const char *language; ///< the language this file contains
    void *data;           ///< user data pointer

    const char *mapped;
    size_t size;
} scan_t;
