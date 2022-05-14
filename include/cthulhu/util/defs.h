#pragma once

#include "cthulhu/util/macros.h"
#include "cthulhu/util/alloc.h"
typedef size_t cerror_t;

#ifdef _WIN32
#    define PATH_SEP "\\"
#else
#    define PATH_SEP "/"
#endif

NODISCARD
char *error_string(alloc_t *alloc, cerror_t error);
