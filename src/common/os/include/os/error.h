#pragma once

#include "core/analyze.h"
#include "core/compiler.h"

#include <stddef.h>

BEGIN_API

/// @defgroup Result OS error handling and result types
/// @ingroup OS
/// @{

/// @brief error code
typedef size_t os_error_t;

/// @brief convert an os error code to a string
///
/// @param error the error code to convert
///
/// @return the string representation of the error code
NODISCARD RET_STRING
const char *os_error_string(os_error_t error);

/// @}

END_API
