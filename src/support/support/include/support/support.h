#pragma once

#include <ctu_support_api.h>

#include "core/analyze.h"

#include "support/loader.h"

#include <stdbool.h>

typedef struct arena_t arena_t;
typedef struct loader_t loader_t;
typedef struct logger_t logger_t;

typedef struct broker_t broker_t;
typedef struct language_runtime_t language_runtime_t;
typedef struct plugin_runtime_t plugin_runtime_t;
typedef struct target_runtime_t target_runtime_t;

CT_BEGIN_API

typedef struct support_t support_t;

/// @brief create a support instance from an existing loader and broker
/// configures the broker with the modules in the loader
RET_NOTNULL
CT_SUPPORT_API support_t *support_new(IN_NOTNULL broker_t *broker, IN_NOTNULL loader_t *loader, IN_NOTNULL arena_t *arena);

/// @brief load all default modules
/// @note does nothing if static modules are not enabled
///
/// @param support the support
CT_SUPPORT_API void support_load_default_modules(IN_NOTNULL support_t *support);

CT_SUPPORT_API bool support_load_module(IN_NOTNULL support_t *support,  module_type_t mask, IN_STRING const char *name);

CT_SUPPORT_API typevec_t *support_get_modules(IN_NOTNULL support_t *support);

CT_SUPPORT_API language_runtime_t *support_get_lang(IN_NOTNULL support_t *support, IN_STRING const char *ext);
CT_SUPPORT_API plugin_runtime_t *support_get_plugin(IN_NOTNULL support_t *support, IN_STRING const char *name);
CT_SUPPORT_API target_runtime_t *support_get_target(IN_NOTNULL support_t *support, IN_STRING const char *name);

CT_END_API
