#pragma once

#include "core/version_def.h"
#include "format/colour.h"

BEGIN_API

typedef struct io_t io_t;

typedef struct format_version_t
{
    format_context_t context;

    io_t *io;

    version_info_t version;
    const char *name;
} format_version_t;

void print_version(format_version_t config);

END_API
