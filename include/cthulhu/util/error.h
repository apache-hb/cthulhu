#pragma once

#ifdef _WIN32
#   define WIN32_LEAN_AND_MEAN
#   define NOMINMAX
#   include "cthulhu/util/giga-windows.h"
#   define ctu_last_error() GetLastError()
typedef DWORD ctu_errno_t;
LPTSTR ctu_err_string(DWORD err);
#else
#   include <errno.h>
#   define ctu_last_error() errno
typedef int ctu_errno_t;
char *ctu_err_string(int err);
#endif
