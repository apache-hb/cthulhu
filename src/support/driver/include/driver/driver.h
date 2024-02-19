#pragma once

#include <ctu_driver_api.h>

#include "cthulhu/broker/broker.h"

/// @defgroup driver Driver helper
/// @brief Driver helper
/// @ingroup support
/// @{

typedef const language_t*(*lang_main_t)(void);
typedef const plugin_t*(*plugin_main_t)(void);
typedef const target_t*(*target_main_t)(void);

#define CT_LANG_ENTRY "lang_main"
#define CT_PLUGIN_ENTRY "plugin_main"
#define CT_TARGET_ENTRY "target_main"

#if CTU_DRIVER_SHARED
#   define CT_LANG_EXPORT(mod) CT_LINKAGE_C CT_EXPORT const language_t *lang_main(void) { return &mod; }
#else
#   define CT_LANG_EXPORT(mod)
#endif

#if CTU_DRIVER_SHARED
#   define CT_PLUGIN_EXPORT(mod) CT_LINKAGE_C CT_EXPORT const plugin_t *plugin_main(void) { return &mod; }
#else
#   define CT_PLUGIN_EXPORT(mod)
#endif

#if CTU_DRIVER_SHARED
#   define CT_TARGET_EXPORT(mod) CT_LINKAGE_C CT_EXPORT const target_t *target_main(void) { return &mod; }
#else
#   define CT_TARGET_EXPORT(mod)
#endif

/// @def CT_LANG_EXPORT(mod)
/// @brief declares the entry point for a language driver module
///
/// conditionally declares the driver entry point for a module to handle building as both
/// a shared and static library.
///
/// @param mod the input module

/// @def CT_PLUGIN_EXPORT(mod)
/// @brief declares the entry point for a plugin driver module
///
/// conditionally declares the driver entry point for a module to handle building as both
/// a shared and static library.
///
/// @param mod the input module

/// @def CT_TARGET_EXPORT(mod)
/// @brief declares the entry point for a target driver module
///
/// conditionally declares the driver entry point for a module to handle building as both
/// a shared and static library.
///
/// @param mod the input module

/// @}
