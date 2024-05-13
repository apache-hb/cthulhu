// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <ctu_cinterop_api.h>

typedef struct io_t io_t;
typedef struct tree_t tree_t;

CT_BEGIN_API

CT_CINTEROP_API void cinterop_emit_header(io_t *io, const tree_t *tree);

CT_END_API
