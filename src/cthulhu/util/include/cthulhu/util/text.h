// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_util_api.h>

#include "core/compiler.h"
#include "core/analyze.h"
#include "core/text.h"

#include <stddef.h>
#include <stdbool.h>

typedef struct logger_t logger_t;
typedef struct node_t node_t;
typedef struct arena_t arena_t;

CT_BEGIN_API

/// @ingroup runtime_util
/// @{

/// @brief escape a string literal into a string
///
/// @param reports the reports to use
/// @param node the node to report errors on
/// @param text the text to escape
/// @param length the length of the text
/// @param arena the arena to allocate the escaped text in
///
/// @return the escaped text
CT_UTIL_API text_t util_text_escape(IN_NOTNULL logger_t *reports, IN_NOTNULL const node_t *node, IN_READS(length) const char *text, size_t length, IN_NOTNULL arena_t *arena);

CT_UTIL_API bool util_text_has_escapes(IN_READS(length) const char *text, size_t length);

/// @}

CT_END_API
