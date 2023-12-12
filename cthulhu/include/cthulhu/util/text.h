#pragma once

#include "core/compiler.h"
#include "core/text.h"

#include <stddef.h>

typedef struct reports_t reports_t;
typedef struct node_t node_t;

BEGIN_API

/// @ingroup RuntimeUtil
/// @{

/// @brief escape a string literal into a string
///
/// @param reports the reports to use
/// @param node the node to report errors on
/// @param text the text to escape
/// @param length the length of the text
///
/// @return the escaped text
text_t util_text_escape(reports_t *reports, const node_t *node, const char *text, size_t length);

/// @}

END_API
