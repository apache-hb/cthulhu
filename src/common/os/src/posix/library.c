// SPDX-License-Identifier: LGPL-3.0-only

#include "os/os.h"

#include "base/panic.h"

#include <dlfcn.h>
#include <errno.h>

os_library_impl_t impl_library_open(const char *path)
{
    return dlopen(path, RTLD_NOW);
}

bool impl_library_close(os_library_impl_t lib)
{
    return dlclose(lib);
}

// casting a object pointer to a function pointer is unspecified behavior.
// gnu warns on it, but this is a posix platform, so its well defined.
#if CT_CC_GNU
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpedantic"
#endif

void *impl_library_symbol(os_library_impl_t lib, const char *name)
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

#if CT_CC_GNU
#   pragma GCC diagnostic pop
#endif
