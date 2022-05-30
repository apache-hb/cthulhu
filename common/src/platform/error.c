// clang-format off
#include "platform/platform.h"
#include "platform/error.h"
// clang-format on

USE_DECL
char *error_string(cerror_t error)
{
    native_cerror_t nativeError = (native_cerror_t)error;
    return native_cerror_to_string(nativeError);
}
