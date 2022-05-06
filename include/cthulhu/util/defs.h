#pragma once

#include "cthulhu/util/macros.h"
typedef size_t error_t;

#ifdef _WIN32
#    define PATH_SEP "\\"
#else
#    define PATH_SEP "/"
#endif

NODISCARD
char *error_string(error_t error);
