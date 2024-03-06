// SPDX-License-Identifier: LGPL-3.0-only

#include "os/os.h"

#include "base/panic.h"

#include <dlfcn.h>
#include <errno.h>

USE_DECL
os_error_t os_library_open(const char *path, os_library_t *library)
{
    CTASSERT(path != NULL);
    CTASSERT(library != NULL);

    library->library = dlopen(path, RTLD_NOW);

    if (library->library == NULL)
    {
        return errno;
    }

    return 0;
}

USE_DECL
os_error_t os_library_close(os_library_t *library)
{
    CTASSERT(library != NULL);

    return dlclose(library->library);
}

// casting a object pointer to a function pointer is unspecified behavior.
// gnu warns on it, but this is a posix platform, so its well defined.
#if CT_CC_GNU
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpedantic"
#endif

USE_DECL
os_error_t os_library_symbol(os_library_t *library, os_symbol_t *symbol, const char *name)
{
    CTASSERT(library != NULL);
    CTASSERT(name != NULL);

    // clear any previous errors
    dlerror();

    os_symbol_t addr = (os_symbol_t)dlsym(library->library, name);

    const char *error = dlerror();
    if (error != NULL)
    {
        return errno;
    }

    *symbol = addr;
    return 0;
}

#if CT_CC_GNU
#   pragma GCC diagnostic pop
#endif
