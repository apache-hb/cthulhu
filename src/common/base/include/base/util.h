#pragma once

#include <ctu_base_api.h>

#include "core/analyze.h"
#include "core/compiler.h"
#include "core/text.h"

#include <stddef.h>

BEGIN_API

/// @ingroup base
/// @{

/// @ingroup base
/// @brief hash a pointer value
///
/// @param ptr the pointer to hash
///
/// @return the hash of the pointer
NODISCARD CONSTFN
CT_BASE_API size_t ptrhash(const void *ptr);

/// @brief create a new owning text array
/// @p text must be at least @p size bytes long
///
/// @param text the text
/// @param size the size of @p text
///
/// @return the text object
CONSTFN
CT_BASE_API text_t text_make(IN_STRING char *text, size_t size);

/// @brief create a new non-owning text array
/// @p text must be at least @p size bytes long
///
/// @param text the text
/// @param size the size of @p text
///
/// @return the text object
CONSTFN
CT_BASE_API text_view_t text_view_make(IN_STRING const char *text, size_t size);

/// @}

END_API
