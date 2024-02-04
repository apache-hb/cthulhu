#pragma once

#include <ctu_base_api.h>

#include "core/analyze.h"
#include "core/compiler.h"
#include "core/text.h"

#include <stddef.h>
#include <stdbool.h>

CT_BEGIN_API

/// @ingroup base
/// @{

/// @brief hash a pointer value
///
/// @param ptr the pointer to hash
///
/// @return the hash of the pointer
CT_NODISCARD CT_CONSTFN
CT_BASE_API size_t ptrhash(const void *ptr);

/// @brief hash a string
///
/// @param str the string to hash
///
/// @return the hash
CT_NODISCARD CT_PUREFN
CT_BASE_API size_t str_hash(IN_STRING const char *str);

/// @brief hash a string with a provided length
///
/// @param text the string to hash
///
/// @return the hash
CT_NODISCARD CT_PUREFN
CT_BASE_API size_t text_hash(text_view_t text);

// stdlib wrappers

/// @brief compare strings equality
///
/// check if 2 strings are equal
///
/// @param lhs the left hand side of the comparison
/// @param rhs the right hand side of the comparison
///
/// @return if the strings are equal
CT_NODISCARD CT_PUREFN
CT_BASE_API bool str_equal(IN_STRING const char *lhs, IN_STRING const char *rhs);

/// @brief get the length of a string not including the null terminator
/// equivalent to strlen but with safety checks
///
/// @pre @p str must not be null
///
/// @param str the string
///
/// @return the length of the string
CT_NODISCARD CT_PUREFN
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
CT_NODISCARD CT_PUREFN
CT_BASE_API int ctu_strncmp(IN_STRING const char *lhs, IN_STRING const char *rhs, size_t length);

/// @brief copy memory from one location to another
/// equivalent to memcpy but with safety checks
///
/// @pre @p dst and @p src must not be null and must be at least @p size bytes long
///
/// @param dst the destination
/// @param src the source
/// @param size the number of bytes to copy
///
/// @return the destination
CT_NOALIAS
CT_BASE_API void *ctu_memcpy(OUT_WRITES(size) void *CT_RESTRICT dst, IN_READS(size) const void *CT_RESTRICT src, size_t size);

// text api

/// @brief create a new owning text array
/// @p text must be a valid string at least @p length bytes long
///
/// @param text the text
/// @param length the length of @p text
///
/// @return the text object
CT_CONSTFN
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
CT_PUREFN
CT_BASE_API text_t text_from(IN_STRING char *text);

/// @brief create a new non-owning text array
/// @p text must be at least @p length bytes long
///
/// @param text the text
/// @param length the length of @p text
///
/// @return the text object
CT_CONSTFN
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
CT_PUREFN
CT_BASE_API text_view_t text_view_from(IN_STRING const char *text);

/// @brief check if two text objects are equal
///
/// @param lhs the left hand side
/// @param rhs the right hand side
///
/// @retval true if @p lhs and @p rhs are equal
/// @retval false otherwise
CT_PUREFN
CT_BASE_API bool text_equal(text_view_t lhs, text_view_t rhs);

/// @}

CT_END_API
