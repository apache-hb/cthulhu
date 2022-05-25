// clang-format off
#include "platform/platform.h"
#include "platform/library.h"
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
