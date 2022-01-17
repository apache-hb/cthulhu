#pragma once

#include "cthulhu/gen/value.h"
#include "cthulhu/backend/gcc/type.h"

gcc_jit_rvalue *rvalue_from_value(gcc_jit_context *context, const value_t *value);
