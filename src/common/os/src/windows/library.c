// SPDX-License-Identifier: LGPL-3.0-only

#include "os_common.h"

CT_LOCAL os_library_impl_t impl_library_open(const char *path)
{
    return LoadLibraryA(path);
}

CT_LOCAL bool impl_library_close(os_library_impl_t lib)
{
    return FreeLibrary(lib) != 0;
}

CT_LOCAL void *impl_library_symbol(os_library_impl_t lib, const char *name)
{
    return (void*)GetProcAddress(lib, name);
}
