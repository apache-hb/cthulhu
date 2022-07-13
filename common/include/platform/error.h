#pragma once

#include "base/analyze.h"

#include <stddef.h>

typedef size_t cerror_t;

NODISCARD
char *error_string(cerror_t error);
