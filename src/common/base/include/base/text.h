#pragma once

#include "core/compiler.h"
#include "core/analyze.h"

#include "core/text.h"

BEGIN_API

/// @ingroup base
/// @{

/// @brief create a new owning text array
/// @p text must be at least @p size bytes long
///
/// @param text the text
/// @param size the size of @p text
///
/// @return the text object
CONSTFN
text_t text_make(IN_STRING char *text, size_t size);

/// @brief create a new non-owning text array
/// @p text must be at least @p size bytes long
///
/// @param text the text
/// @param size the size of @p text
///
/// @return the text object
CONSTFN
text_view_t text_view_make(IN_STRING const char *text, size_t size);

/// @}

END_API
