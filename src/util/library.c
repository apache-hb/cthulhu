#define I_WILL_BE_INCLUDING_PLATFORM_CODE

// clang-format off
#include "src/platform/platform.h"
#include "cthulhu/util/library.h"
// clang-format on

USE_DECL
library_t library_open(const char *path, cerror_t *error)
{
    native_cerror_t nativeError = 0;
    library_t result = native_library_open(path, &nativeError);
    *error = nativeError;
    return result;
}

void library_close(library_t library)
{
    native_library_close(library);
}

USE_DECL
void *library_get(library_t library, const char *symbol, cerror_t *error)
{
    native_cerror_t nativeError = 0;
    void *result = native_library_get_symbol(library, symbol, &nativeError);
    *error = nativeError;
    return result;
}
