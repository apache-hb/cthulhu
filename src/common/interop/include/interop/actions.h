// SPDX-License-Identifier: LGPL-3.0-only
#pragma once

#include <ctu_interop_api.h>

#include "core/analyze.h"

CT_BEGIN_API

typedef struct where_t where_t;
typedef struct scan_t scan_t;

/// @defgroup flex_bison_macros Helpers for flex and bison driver frontends
/// @ingroup interop
/// @{

/// @brief tracks the current source position
/// @see https://ftp.gnu.org/old-gnu/Manuals/flex-2.5.4/html_node/flex_14.html
///
/// @param where a pointer to the current location
/// @param text current source text
CT_INTEROP_API void flex_action(INOUT_NOTNULL where_t *where, IN_STRING const char *text);

/// @brief retrevies more input for flex
///
/// @param scan the source scanner
/// @param out output buffer to write to
/// @param size total number of characters to write
///
/// @return number of characters written
CT_INTEROP_API int flex_input(INOUT_NOTNULL scan_t *scan, OUT_WRITES(size) char *out, int size);

/// @brief initialize source location tracking
///
/// @param where the source location to initialize
CT_INTEROP_API void flex_init(OUT_NOTNULL where_t *where);

/// @brief update the source location
///
/// @param where the source location to update
/// @param offsets the source location offsets
/// @param steps the number of steps to update by
CT_INTEROP_API void flex_update(INOUT_NOTNULL where_t *where, IN_READS(steps) const where_t *offsets, int steps);

/// @}

CT_END_API
