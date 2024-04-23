// SPDX-License-Identifier: LGPL-3.0-only

#include "os_common.h"

os_library_impl_t impl_library_open(const char *path)
{
    return LoadLibraryA(path);
}

bool impl_library_close(os_library_impl_t lib)
{
    return FreeLibrary(lib) != 0;
}

void *impl_library_symbol(os_library_impl_t lib, const char *name)
{
    return (void*)GetProcAddress(lib, name);
}
