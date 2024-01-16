#pragma once

#include <ctu_driver_api.h>

/// @defgroup driver Driver helper
/// @brief Driver helper
/// @ingroup runtime
/// @{

#if CTU_BUILD_SHARED
#   define CTU_DRIVER_ENTRY(mod) CT_DRIVER_API const language_t *lang_main(void) { return &mod; }
#else
#   define CTU_DRIVER_ENTRY(mod)
#endif

/// @def CTU_DRIVER_ENTRY(mod)
/// @brief declares the entry point for a driver module
///
/// conditionally declares the driver entry point for a module to handle building as both
/// a shared and static library.
///
/// @param mod the input module

/// @}
