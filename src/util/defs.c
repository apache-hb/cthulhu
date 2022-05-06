#define I_WILL_BE_INCLUDING_PLATFORM_CODE

// clang-format off
#include "src/platform/platform.h"
#include "cthulhu/util/defs.h"
// clang-format on

USE_DECL
char *error_string(error_t error)
{
    native_error_t nativeError = (native_error_t)error;
    return native_error_to_string(nativeError);
}
