// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <ctu_demangle_api.h>

#include "core/text.h"

#include <limits.h>

typedef struct arena_t arena_t;

CT_BEGIN_API

typedef struct ms_demangle_t ms_demangle_t;

typedef unsigned ms_node_index_t;
typedef unsigned ms_buffer_offset_t;

#define MS_DEMANGLE_INVALID UINT_MAX

typedef enum ms_demangle_node_type_t {
#define MS_DEMANGLE_NODE(ID, STR) ID,
#include "demangle/demangle.inc"

    eMsDemangleNodeCount
} ms_demangle_node_type_t;

typedef struct buffer_range_t {
    ms_buffer_offset_t front;
    ms_buffer_offset_t back;
} buffer_range_t;

typedef struct ms_demangle_node_t {
    ms_demangle_node_type_t type;
    buffer_range_t source;
} ms_demangle_node_t;

CT_DEMANGLE_API ms_node_index_t ms_demangle(text_view_t text, arena_t *arena);

CT_END_API
