// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_std_api.h>

#include "core/analyze.h"
#include "core/compiler.h"
#include "core/text.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

CT_BEGIN_API

typedef struct arena_t arena_t;
typedef struct vector_t vector_t;
typedef struct map_t map_t;

/// @defgroup string_utils String manipulation and processing
/// @ingroup standard
/// @brief String manipulation and processing
/// @{

/// @brief format a string with printf-like syntax
///
/// format a string with a va_list and printf-like syntax into a text buffer.
///
/// @pre @p str is a pointer to a buffer of at least @p len bytes
/// @pre @p fmt is a valid format string
///
/// @param str the buffer to format into
/// @param len the length of the buffer
/// @param fmt the format string
/// @param args the arguments to format
///
/// @return the number of characters written
CT_STD_API size_t str_vsprintf(STA_WRITES(len) char *str, size_t len, IN_STRING const char *fmt, va_list args);

/// @brief format a string with printf-like syntax
///
/// format a string with printf-like syntax into a text buffer.
///
/// @pre @p str is a pointer to a buffer of at least @p len bytes
/// @pre @p fmt is a valid format string
///
/// @param str the buffer to format into
/// @param len the length of the buffer
/// @param fmt the format string
/// @param ... the arguments to format
///
/// @return the number of characters written
STA_PRINTF(3, 4)
CT_STD_API size_t str_sprintf(STA_WRITES(len) char *str, size_t len, STA_FORMAT_STRING const char *fmt, ...);

/// @brief format a string
///
/// format a string with printf-like syntax into a text buffer.
/// useful when you need the length of the formatted string.
///
/// @param arena the arena to allocate the formatted string in
/// @param fmt the format string
/// @param ... the arguments to format
///
/// @return the formatted string
CT_NODISCARD STA_PRINTF(2, 3)
CT_STD_API text_t text_format(IN_NOTNULL arena_t *arena, STA_FORMAT_STRING const char *fmt, ...);

/// @brief format a string
///
/// format a string with a va_list and printf-like syntax into a text buffer.
/// useful when you need the length of the formatted string.
///
/// @param arena the arena to allocate the formatted string in
/// @param fmt the format string
/// @param args the arguments to format
///
/// @return the formatted string
CT_NODISCARD
CT_STD_API text_t text_vformat(IN_NOTNULL arena_t *arena, IN_STRING const char *fmt, va_list args);

/// @brief format a string
///
/// format a string with printf-like syntax
///
/// @param arena the arena to allocate the formatted string in
/// @param fmt the format string
/// @param ... the arguments to format
///
/// @return the formatted string
CT_NODISCARD STA_PRINTF(2, 3)
CT_STD_API char *str_format(IN_NOTNULL arena_t *arena, STA_FORMAT_STRING const char *fmt, ...);

/// @brief format a string
///
/// format a string with a va_list and printf-like syntax
///
/// @param arena the arena to allocate the formatted string in
/// @param fmt the format string
/// @param args the arguments to format
///
/// @return the formatted string
CT_NODISCARD
CT_STD_API char *str_vformat(IN_NOTNULL arena_t *arena, IN_STRING const char *fmt, va_list args);

/// @brief see if a string starts with a prefix
///
/// @param str the string to search
/// @param prefix the prefix to check for
///
/// @return if str starts with prefix
CT_NODISCARD CT_PUREFN
CT_STD_API bool str_startswith(IN_STRING const char *str, IN_STRING const char *prefix);

/// @brief check if a string ends with a substring
///
/// @param str the string to search
/// @param suffix the suffix to check for
///
/// @return if str ends with suffix
CT_NODISCARD CT_PUREFN
CT_STD_API bool str_endswith(IN_STRING const char *str, IN_STRING const char *suffix);

/// @brief check if a string ends with a substring
///
/// equivalent to str_endswith but with a length parameter
///
/// @param str the string to search
/// @param len the length of the string
/// @param suffix the suffix to check for
///
/// @return if str ends with suffix
CT_NODISCARD CT_PUREFN
CT_STD_API bool str_endswithn(STA_READS(len) const char *str, size_t len, IN_STRING const char *suffix);

/// @brief join strings
///
/// join a vector of strings together with a separator
///
/// @param sep the separator to use
/// @param parts a vector of strings to join
/// @param arena the arena to allocate the joined string in
///
/// @return the joined string
CT_NODISCARD
CT_STD_API char *str_join(IN_STRING const char *sep, IN_NOTNULL const vector_t *parts, IN_NOTNULL arena_t *arena);

/// @brief repeat a string
///
/// repeat a string n times
///
/// @param str the string to repeat
/// @param times the number of times to repeat
/// @param arena the arena to allocate the repeated string in
///
/// @return the repeated string
CT_NODISCARD
CT_STD_API char *str_repeat(IN_STRING const char *str, size_t times, IN_NOTNULL arena_t *arena);

/// @brief turn a string into a C string literal
///
/// normalize a string into a valid C string literal
///
/// @param str the string to normalize
/// @param arena the arena to allocate the normalized string in
///
/// @return the normalized string
CT_NODISCARD
CT_STD_API char *str_normalize(IN_STRING const char *str, IN_NOTNULL arena_t *arena);

/// @brief turn a string with length into a C string literal
///
/// normalize a string with length into a valid C string literal
///
/// @param text the text to normalize
/// @param arena the arena to allocate the normalized string in
///
/// @return the normalized string
CT_NODISCARD
CT_STD_API char *str_normalizen(text_view_t text, IN_NOTNULL arena_t *arena);

/// @brief normalize a string into a buffer
///
/// normalize a string into a buffer. if @p dst is NULL, the length of the
/// normalized string is returned.
///
/// @param dst the buffer to write the normalized string into
/// @param dstlen the length of the buffer
/// @param src the string to normalize
/// @param srclen the length of the string
///
/// @return the number of characters written
CT_STD_API size_t str_normalize_into(
    STA_WRITES(len) char *dst,
    size_t dstlen,
    STA_READS(srclen) const char *src,
    size_t srclen);

/// @brief split a string into a vector by a separator
///
/// @note the seperator is not included in the resulting substrings.
/// @note if no separator is found, the entire string is returned in the vectors
/// first element.
///
/// @param str the string to split
/// @param sep the separator to split by
/// @param arena the arena to allocate the substrings in
///
/// @return the substrings
CT_NODISCARD
CT_STD_API vector_t *str_split(IN_STRING const char *str, IN_STRING const char *sep, IN_NOTNULL arena_t *arena);

/// @brief find the longest common prefix of a vector of paths
///
/// @note if no common prefix is found, the empty string is returned.
///
/// @param args the vector of paths to find the common prefix of
/// @param arena the arena to allocate the common prefix in
///
/// @return the common prefix
CT_NODISCARD RET_NOTNULL
CT_STD_API const char *str_common_prefix(IN_NOTNULL const vector_t *args, IN_NOTNULL arena_t *arena);

/// @brief find the last instance of a substring in a string
///
/// @param str the string to search
/// @param sub the substring to search for
///
/// @return the index of the last instance of @p sub in @p str, or @a SIZE_MAX if
/// sub is not found
RET_INSPECT CT_PUREFN
CT_STD_API size_t str_rfind(IN_STRING const char *str, IN_STRING const char *sub);

/// @brief find the first instance of a substring in a string
///
/// @param str the string to search
/// @param sub the substring to search for
///
/// @return the index of the first instance of @p sub in @p str, or @a SIZE_MAX if @p sub is not found
RET_INSPECT CT_PUREFN
CT_STD_API size_t str_find(IN_STRING const char *str, IN_STRING const char *sub);

/// @brief find the first instance of a set of characters in a string
///
/// @param str the string to search
/// @param letters the characters to search for
///
/// @return the index of the first instance of any character in @p letters in @p str, or @a SIZE_MAX if no character is found
RET_INSPECT CT_PUREFN
CT_STD_API size_t str_rfind_any(IN_STRING const char *str, IN_STRING const char *letters);

/// @brief check if a character is any of a set of characters
///
/// @param c the character to check
/// @param chars the characters to check against
///
/// @retval true @p c is any of @p chars
/// @retval false @p c is not any of @p chars
CT_NODISCARD CT_PUREFN
CT_STD_API bool char_is_any_of(char c, IN_STRING const char *chars);

/// @brief check if a string contains a substring
///
/// @param str the string to search
/// @param search the substring to search for
///
/// @return if @p sub is found in @p str
CT_NODISCARD CT_PUREFN
CT_STD_API bool str_contains(IN_STRING const char *str, IN_STRING const char *search);

/// @brief replace all instances of a substring in a string
///
/// @param str the string to replace elements in
/// @param search the substring to replace
/// @param repl the replacement substring
/// @param arena the arena to allocate the new string in
///
/// @return a copy of @p str with all instances of @p search replaced with @p repl
CT_NODISCARD
CT_STD_API char *str_replace(IN_STRING const char *str, IN_STRING const char *search, IN_STRING const char *repl, IN_NOTNULL arena_t *arena);

/// @brief replace all instances of a substring in a string in place
///
/// @param text the text to replace elements in
/// @param search the substring to replace
/// @param repl the replacement substring
CT_STD_API void str_replace_inplace(INOUT_NOTNULL text_t *text, IN_STRING const char *search, IN_STRING const char *repl);

/// @brief trim chars from the back of a string in place
///
/// @param text the text to trim
/// @param chars the characters to trim
CT_STD_API void str_trim_back_inplace(INOUT_NOTNULL text_t *text, IN_STRING const char *chars);

/// @brief sort a strings contents in place
///
/// @param str the string to sort
/// @param len the length of the string
CT_STD_API void str_sort_inplace(STA_UPDATES(len) char *str, size_t len);

/// @brief replace all instances of a each substring in a string with provided replacement
///
/// @param str the string to replace elements in
/// @param repl a map of substrings to replace and their replacements
/// @param arena the arena to allocate the new string in
///
/// @return a copy of @p str with all instances of substrings in @p repl replaced
CT_NODISCARD
CT_STD_API char *str_replace_many(IN_STRING const char *str, IN_NOTNULL const map_t *repl, IN_NOTNULL arena_t *arena);

/// @brief remove all instances of @p letters from @p str
///
/// @param str the string to erase letters from
/// @param len the length of @p str
/// @param letters the letters to erase
///
/// @return @p str with all instances of @p letters removed
CT_NODISCARD CT_NOALIAS
CT_STD_API char *str_erase(STA_READS(len) char *str, size_t len, IN_STRING const char *letters);

/// @brief get the filename from a path
///
/// @param path the path to get the filename from
/// @param arena the arena to allocate the filename in
///
/// @return the filename extracted from @p path
CT_NODISCARD
CT_STD_API char *str_basename(IN_STRING const char *path, IN_NOTNULL arena_t *arena);

/// @brief get the filename from a path
///
/// @param path the path to get the filename from
/// @param arena the arena to allocate the filename in
///
/// @return the filename extracted from @p path
CT_NODISCARD
CT_STD_API char *str_filename(IN_STRING const char *path, IN_NOTNULL arena_t *arena);

/// @brief remove the last file extension from a path
///
/// @param path the path to remove the extension from
/// @param arena the arena to allocate the path in
///
/// @return the @p path with the last extension removed
CT_NODISCARD
CT_STD_API char *str_noext(IN_STRING const char *path, IN_NOTNULL arena_t *arena);

/// @brief get the last file extension from a path
///
/// @param path the path to get the extension from
/// @param arena the arena to allocate the extension in
///
/// @return the last extension in @p path
CT_NODISCARD
CT_STD_API char *str_ext(IN_STRING const char *path, IN_NOTNULL arena_t *arena);

/// @brief get the directory segment of a path
///
/// @param path the path to get the directory from
/// @param arena the arena to allocate the directory in
///
/// @return the directory extracted from @p path
CT_NODISCARD
CT_STD_API char *str_directory(IN_STRING const char *path, IN_NOTNULL arena_t *arena);

/// @brief uppercase an ascii string
///
/// @param str the string
/// @param arena the arena to allocate the uppercase string in
///
/// @return @p str with all lowercase charaters replaced with uppercase
CT_NODISCARD
CT_STD_API char *str_upper(IN_STRING const char *str, IN_NOTNULL arena_t *arena);

/// @brief lowercase an ascii string
/// this allocates a new string in the provided arena
///
/// @param str the string
/// @param arena the arena to allocate the lowercase string in
///
/// @return @p str with all uppercase charaters replaced with lowercase
CT_NODISCARD
CT_STD_API char *str_lower(IN_STRING const char *str, IN_NOTNULL arena_t *arena);

/// @brief get the lowercase version of a character
///
/// @param c the character
///
/// @return the lowercase version of @p c, or the character itself
CT_NODISCARD CT_CONSTFN
CT_STD_API char str_tolower(char c);

/// @brief get the uppercase version of a character
///
/// @param c the character
///
/// @return the uppercase version of @p c or the character itself
CT_NODISCARD CT_CONSTFN
CT_STD_API char str_toupper(char c);

/// @brief check if two strings are equal
///
/// @param lhs the left hand side of the comparison
/// @param rhs the right hand side of the comparison
///
/// @return if the strings are equal
CT_NODISCARD
CT_STD_API bool text_view_equal(text_view_t lhs, text_view_t rhs);

#define STR_WHITESPACE " \t\r\v\n\f" ///< all whitespace charaters

/// @}

CT_END_API
