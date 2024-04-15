// SPDX-License-Identifier: LGPL-3.0-only

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

/// @brief check if a path is special
/// special paths are paths such as "." and ".." that are not valid for most operations
///
/// @param path the path to check
///
/// @retval true if the path is special
CT_NODISCARD CT_CONSTFN
CT_BASE_API bool is_path_special(IN_STRING const char *path);

/// @brief hash a pointer value
///
/// @param ptr the pointer to hash
///
/// @return the hash of the pointer
CT_NODISCARD CT_CONSTFN
CT_BASE_API size_t ctu_ptrhash(const void *ptr);

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

/// @brief copy a string
/// equivalent to strcpy but with safety checks
///
/// @pre @p dst and @p src must not be null and @p dst must be at least @p size bytes long
///
/// @param dst the destination
/// @param src the source
/// @param size the size of @p dst
///
/// @return the destination
CT_BASE_API char *ctu_strcpy(OUT_WRITES(size) char *dst, IN_STRING const char *src, size_t size);

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

/// @brief check if a string is empty
/// equivalent to strlen(str) == 0
///
/// @pre @p str must not be null
///
/// @param str the string
///
/// @retval true if the string is empty
/// @retval false otherwise
CT_NODISCARD CT_PUREFN
CT_BASE_API bool ctu_string_empty(IN_STRING const char *str);

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

/// @brief compare two strings
/// equivalent to strcmp but with safety checks
///
/// @pre @p lhs and @p rhs must not be null
///
/// @param lhs the left hand side
/// @param rhs the right hand side
///
/// @return the comparison result
CT_NODISCARD CT_PUREFN
CT_BASE_API int ctu_strcmp(IN_STRING const char *lhs, IN_STRING const char *rhs);

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

/// @brief move memory from one location to another
/// equivalent to memmove but with safety checks
///
/// @pre @p dst and @p src must not be null and must be at least @p size bytes long
///
/// @param dst the destination
/// @param src the source
/// @param size the number of bytes to move
///
/// @return the destination
CT_BASE_API void *ctu_memmove(OUT_WRITES(size) void *dst, IN_READS(size) const void *src, size_t size);

/// @brief set memory to a value
/// equivalent to memset but with safety checks
///
/// @pre @p dst must not be null and must be at least @p size bytes long
///
/// @param dst the destination
/// @param value the value to set
/// @param size the number of bytes to set
CT_NOALIAS
CT_BASE_API void ctu_memset(OUT_WRITES(size) void *dst, int value, size_t size);

/// @brief find a substring in a string
/// equivalent to strstr but with safety checks
///
/// @pre @p haystack and @p needle must not be null
///
/// @param haystack the string to search in
/// @param needle the string to search for
///
/// @return the position of @p needle in @p haystack or null if not found
CT_NODISCARD CT_PUREFN
CT_BASE_API char *ctu_strstr(IN_STRING const char *haystack, IN_STRING const char *needle);

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
