// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "core/compiler.h"

typedef struct logger_t logger_t;
typedef struct vector_t vector_t;

CT_BEGIN_API

typedef struct tree_cookie_t {
    logger_t *reports;
    vector_t *stack;
} tree_cookie_t;

CT_END_API
