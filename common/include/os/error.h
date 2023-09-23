#pragma once

#include "base/analyze.h"

#include <stddef.h>

BEGIN_API

// error api

typedef struct os_result_t os_result_t;
typedef size_t os_error_t;

// documentation macros

#define OS_RESULT(TYPE) os_result_t *

// result & error api

NODISCARD
os_error_t os_error(os_result_t *result);

NODISCARD
void *os_value(os_result_t *result);

NODISCARD
const char *os_decode(os_error_t error);

#define OS_VALUE(TYPE, RESULT) (*(TYPE *)os_value(RESULT))
#define OS_VALUE_OR(TYPE, RESULT, OTHER) (os_error(RESULT) ? (OTHER) : OS_VALUE(TYPE, RESULT))

END_API
