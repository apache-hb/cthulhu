#pragma once

#include <ctu_base_api.h>

#include "core/analyze.h"
#include "core/compiler.h"
#include "core/text.h"

#include <stddef.h>
#include <stdbool.h>

BEGIN_API

/// @ingroup base
/// @{

/// @brief hash a pointer value
///
/// @param ptr the pointer to hash
///
/// @return the hash of the pointer
NODISCARD CONSTFN
CT_BASE_API size_t ptrhash(const void *ptr);

/// @brief get the length of a string not including the null terminator
/// equivalent to strlen but with safety checks
///
/// @pre @p str must not be null
///
/// @param str the string
///
/// @return the length of the string
NODISCARD CONSTFN
CT_BASE_API size_t ctu_strlen(IN_STRING const char *str);

/// @brief compare two strings
/// equivalent to strncmp but with safety checks
///
/// @pre @p lhs and @p rhs must not be null
///
/// @param lhs the left hand side
/// @param rhs the right hand side
/// @param length the length to compare
///
/// @return the comparison result
NODISCARD CONSTFN
CT_BASE_API int ctu_strncmp(IN_STRING const char *lhs, IN_STRING const char *rhs, size_t length);

/// @brief create a new owning text array
/// @p text must be a valid string at least @p length bytes long
///
/// @param text the text
/// @param length the length of @p text
///
/// @return the text object
CONSTFN
CT_BASE_API text_t text_make(IN_STRING char *text, size_t length);

/// @brief create a new owning text array
/// this is a shortcut for
/// @code{.c}
/// text_make(text, ctu_strlen(text))
/// @endcode
///
/// @param text the text
///
/// @return the text object
CONSTFN
CT_BASE_API text_t text_from(IN_STRING char *text);

/// @brief create a new non-owning text array
/// @p text must be at least @p length bytes long
///
/// @param text the text
/// @param length the length of @p text
///
/// @return the text object
CONSTFN
CT_BASE_API text_view_t text_view_make(IN_STRING const char *text, size_t length);

/// @brief create a new non-owning text array
/// this is a shortcut for
/// @code{.c}
/// text_view_make(text, ctu_strlen(text))
/// @endcode
///
/// @param text the text
///
/// @return the text object
CONSTFN
CT_BASE_API text_view_t text_view_from(IN_STRING const char *text);

/// @brief check if two text objects are equal
///
/// @param lhs the left hand side
/// @param rhs the right hand side
///
/// @retval true if @p lhs and @p rhs are equal
/// @retval false otherwise
CONSTFN
CT_BASE_API bool text_equal(text_view_t lhs, text_view_t rhs);

/// @}

END_API
