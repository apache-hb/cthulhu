#pragma once

#include <stddef.h>
typedef size_t error_t;

#ifdef _WIN32
#    define PATH_SEP "\\"
#else
#    define PATH_SEP "/"
#endif