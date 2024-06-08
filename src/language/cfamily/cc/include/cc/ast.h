// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

typedef struct c_ast_t c_ast_t;

typedef enum c_kind_t
{
#define CC_TYPE(id, name) id,
#include "cc/cc.inc"

    eCCount
} c_kind_t;
