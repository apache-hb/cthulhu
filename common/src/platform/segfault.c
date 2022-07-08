// clang-format off
#include "platform/platform.h"
#include "platform/segfault.h"
// clang-format on

// if addrsan was enabled we should its handlers instead
void install_segfault(void)
{
#if !ADDRSAN_ENABLED
    native_install_segfault();
#endif
}
