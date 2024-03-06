// SPDX-License-Identifier: LGPL-3.0-only

#include "os/os.h"

#include "base/panic.h"

#if CTU_WIN32_TRICKERY
#   include <winbase.h>
#endif

USE_DECL
os_error_t os_library_open(const char *path, os_library_t *library)
{
    CTASSERT(path != NULL);
    CTASSERT(library != NULL);

    library->library = LoadLibraryA(path);
    if (library->library == NULL)
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}

USE_DECL
os_error_t os_library_close(os_library_t *library)
{
    CTASSERT(library != NULL);

    if (!FreeLibrary(library->library))
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}

USE_DECL
os_error_t os_library_symbol(os_library_t *library, os_symbol_t *symbol, const char *name)
{
    CTASSERT(library != NULL);
    CTASSERT(name != NULL);

    os_symbol_t addr = (os_symbol_t)GetProcAddress(library->library, name);
    if (addr == NULL)
    {
        return GetLastError();
    }

    *symbol = addr;
    return ERROR_SUCCESS;
}
