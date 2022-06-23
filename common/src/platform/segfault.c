// clang-format off
#include "platform/platform.h"
#include "platform/segfault.h"
// clang-format on

#ifndef __has_feature
#   define __has_feature(...) 0
#endif

#define ADDRSAN_ENABLED ((__SANITIZE_ADDRESS__ != 0) || __has_feature(address_sanitizer))

// if addrsan was enabled we should its handlers instead
void install_segfault(void)
{
#if !ADDRSAN_ENABLED
    native_install_segfault();   
#endif
}
