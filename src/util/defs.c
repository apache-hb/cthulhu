#define I_WILL_BE_INCLUDING_PLATFORM_CODE

// clang-format off
#include "src/platform/platform.h"
#include "cthulhu/util/defs.h"
// clang-format on

USE_DECL
char *error_string(cerror_t error)
{
    native_cerror_t nativeError = (native_cerror_t)error;
    return native_cerror_to_string(nativeError);
}
