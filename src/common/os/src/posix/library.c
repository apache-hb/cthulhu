// SPDX-License-Identifier: LGPL-3.0-only

#include "os/os.h"

#include "base/panic.h"

#include <dlfcn.h>
#include <errno.h>

CT_LOCAL os_library_impl_t impl_library_open(const char *path)
{
    return dlopen(path, RTLD_NOW);
}

CT_LOCAL bool impl_library_close(os_library_impl_t lib)
{
    return dlclose(lib);
}

// casting a object pointer to a function pointer is unspecified behavior.
// gnu warns on it, but this is a posix platform, so its well defined.
#if defined(__GNUC__) && !defined(__clang__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpedantic"
#endif

CT_LOCAL void *impl_library_symbol(os_library_impl_t lib, const char *name)
{
    // clear any previous errors
    dlerror();

    void *addr = (void*)dlsym(lib, name);

    const char *error = dlerror();
    if (error != NULL)
    {
        return NULL;
    }

    return addr;
}

#if defined(__GNUC__) && !defined(__clang__)
#   pragma GCC diagnostic pop
#endif
