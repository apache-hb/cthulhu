// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <ctu_cinterop_api.h>

#include "cthulhu/tree/ops.h"

typedef struct io_t io_t;
typedef struct tree_t tree_t;

CT_BEGIN_API

typedef struct cheader_t cheader_t;

typedef struct ctype_t ctype_t;

CT_CINTEROP_API void cinterop_emit_header(io_t *io, const tree_t *tree);

CT_CINTEROP_API ctype_t *ctype_digit(digit_t digit, sign_t sign);

CT_END_API
