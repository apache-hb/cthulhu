#include "cthulhu/util/defs.h"

#define I_WILL_BE_INCLUDING_PLATFORM_CODE
#include "src/platform/platform.h"

char *error_string(error_t error)
{
    native_error_t nativeError = error;
    return native_error_to_string(nativeError);
}
