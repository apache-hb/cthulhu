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
void os_library_close(os_library_t *library)
{
    CTASSERT(library != NULL);

    dlclose(library->library);
}

// casting a object pointer to a function pointer is unspecified behavior
// gnu warns on it, but posix requires it so we disable the warning
#if CC_GNU
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpedantic"
#endif

USE_DECL
os_fn_t os_library_symbol(os_library_t *library, const char *name)
{
    CTASSERT(library != NULL);
    CTASSERT(name != NULL);

    return (void(*)(void))dlsym(library->library, name);
}

#if CC_GNU
#   pragma GCC diagnostic pop
#endif
