#pragma once

#include "core/version_def.h"

#include "format/format.h"

BEGIN_API

typedef struct print_version_t
{
    print_options_t options;

    version_info_t version;
    const char *name;
} print_version_t;

void print_version(print_version_t config);

END_API
