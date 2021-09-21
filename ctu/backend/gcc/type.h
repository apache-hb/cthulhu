#pragma once

#include <libgccjit.h>

#include "ctu/type/type.h"

gcc_jit_type *select_gcc_type(gcc_jit_context *ctx, const type_t *type);
