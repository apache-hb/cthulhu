#pragma once

#include <ctu_driver_api.h>

#if CTU_BUILD_SHARED
#   define CTU_DRIVER_ENTRY(mod) CT_DRIVER_API const language_t *lang_main(void) { return &mod; }
#else
#   define CTU_DRIVER_ENTRY(mod)
#endif
