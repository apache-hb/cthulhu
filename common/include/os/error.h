#pragma once

#include "base/analyze.h"

#include <stddef.h>

BEGIN_API

/// @defgroup Error Handling
/// @brief error handling and result types
/// @{

/// @brief result type
typedef struct os_result_t os_result_t;

/// @brief error code
typedef size_t os_error_t;

/// @def OS_RESULT(TYPE)
/// @brief the result type for a given type
/// @param TYPE the type this result object contains on success
#define OS_RESULT(TYPE) os_result_t *

/// @brief get the error code from a result
///
/// @param result the result to get the error code from
///
/// @return the error code
NODISCARD
os_error_t os_error(os_result_t *result);

/// @brief get the value from a result
///
/// @param result the result to get the value from
///
/// @return the value
NODISCARD
void *os_value(os_result_t *result);

/// @brief convert an os error code to a string
///
/// @param error the error code to convert
///
/// @return the string representation of the error code
NODISCARD
const char *os_error_string(os_error_t error);

/// @def OS_VALUE(TYPE, RESULT)
/// @brief get the value from a result
/// @note always ensure the result is not an error before using this macro
///
/// @param TYPE the type of the value
/// @param RESULT the result to get the value from
#define OS_VALUE(TYPE, RESULT) (*(TYPE *)os_value(RESULT))

/// @def OS_VALUE_OR(TYPE, RESULT, OTHER)
/// @brief get the value from a result or return a default value
///
/// @param TYPE the type of the value
/// @param RESULT the result to get the value from
/// @param OTHER the default value to return if the result is an error
#define OS_VALUE_OR(TYPE, RESULT, OTHER) (os_error(RESULT) ? (OTHER) : OS_VALUE(TYPE, RESULT))

/// @}

END_API
