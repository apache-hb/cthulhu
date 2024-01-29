#pragma once

#include <ctu_base_api.h>

#include "core/compiler.h"
#include "core/analyze.h"

#include <stdarg.h>
#include <stdbool.h>

CT_BEGIN_API

/// @defgroup log Verbose logging
/// @ingroup base
/// @brief Verbose logging
/// @{

/// @brief a logging callback
typedef void (*verbose_t)(const char *fmt, va_list args);

/// @brief the global verbose logging callback
CT_BASE_API extern verbose_t gVerboseCallback;

/// @brief update the verbosity of the logging system
///
/// @param enable if verbose logging should be enabled
CT_BASE_API void ctu_log_update(bool enable);

/// @brief check if verbose logging is enabled
///
/// @return if verbose logging is enabled
CT_BASE_API bool ctu_log_enabled(void);

/// @brief log a message
///
/// @param fmt the format string
/// @param ... the format arguments
CT_PRINTF(1, 2)
CT_BASE_API void ctu_log(CT_FMT_STRING const char *fmt, ...);

/// @brief log a message
///
/// @param fmt the format string
/// @param args the format arguments
CT_BASE_API void ctu_vlog(IN_STRING const char *fmt, va_list args);

/// @}

CT_END_API
