#pragma once

#include "ctu/gen/value.h"
#include "ctu/backend/gcc/type.h"

gcc_jit_rvalue *rvalue_from_value(gcc_jit_context *context, const value_t *value);
void *blob_from_value(const value_t *value, size_t *size);
