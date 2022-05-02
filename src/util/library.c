#include "cthulhu/util/library.h"

#define I_WILL_BE_INCLUDING_PLATFORM_CODE

#include "src/platform/platform.h"

library_t library_open(const char *path, error_t *error)
{
    native_error_t nativeError = 0;
    library_t result = native_library_open(path, &nativeError);
    *error = nativeError;
    return result;
}

void library_close(library_t library)
{
    native_library_close(library);
}

void *library_get(library_t library, const char *symbol, error_t *error)
{
    native_error_t nativeError = 0;
    void *result = native_library_get_symbol(library, symbol, &nativeError);
    *error = nativeError;
    return result;
}
