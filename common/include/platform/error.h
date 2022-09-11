#pragma once

#include "base/analyze.h"

#include <stddef.h>

typedef size_t cerror_t;

void platform_init(void);

NODISCARD
char *error_string(cerror_t error);
