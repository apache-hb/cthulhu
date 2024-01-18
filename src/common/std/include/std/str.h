#pragma once

#include <ctu_std_api.h>

#include "core/analyze.h"
#include "core/compiler.h"
#include "core/text.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

BEGIN_API

typedef struct arena_t arena_t;
typedef struct vector_t vector_t;
typedef struct map_t map_t;

/// @defgroup string_utils String manipulation and processing
/// @ingroup standard
/// @brief String manipulation and processing
/// @{

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
NODISCARD CT_PRINTF(2, 3)
CT_STD_API text_t text_format(IN_NOTNULL arena_t *arena, FMT_STRING const char *fmt, ...);

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
NODISCARD
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
NODISCARD CT_PRINTF(2, 3)
CT_STD_API char *str_format(IN_NOTNULL arena_t *arena, FMT_STRING const char *fmt, ...);

/// @brief format a string
///
/// format a string with a va_list and printf-like syntax
///
/// @param arena the arena to allocate the formatted string in
/// @param fmt the format string
/// @param args the arguments to format
///
/// @return the formatted string
NODISCARD
CT_STD_API char *str_vformat(IN_NOTNULL arena_t *arena, IN_STRING const char *fmt, va_list args);

/// @brief see if a string starts with a prefix
///
/// @param str the string to search
/// @param prefix the prefix to check for
///
/// @return if str starts with prefix
NODISCARD PUREFN
CT_STD_API bool str_startswith(IN_STRING const char *str, IN_STRING const char *prefix);

/// @brief check if a string ends with a substring
///
/// @param str the string to search
/// @param suffix the suffix to check for
///
/// @return if str ends with suffix
NODISCARD PUREFN
CT_STD_API bool str_endswith(IN_STRING const char *str, IN_STRING const char *suffix);

/// @brief join strings
///
/// join a vector of strings together with a separator
///
/// @param sep the separator to use
/// @param parts a vector of strings to join
/// @param arena the arena to allocate the joined string in
///
/// @return the joined string
NODISCARD
CT_STD_API char *str_join(IN_STRING const char *sep, IN_NOTNULL vector_t *parts, IN_NOTNULL arena_t *arena);

/// @brief repeat a string
///
/// repeat a string n times
///
/// @param str the string to repeat
/// @param times the number of times to repeat
/// @param arena the arena to allocate the repeated string in
///
/// @return the repeated string
NODISCARD
CT_STD_API char *str_repeat(IN_STRING const char *str, size_t times, IN_NOTNULL arena_t *arena);

/// @brief turn a string into a C string literal
///
/// normalize a string into a valid C string literal
///
/// @param str the string to normalize
/// @param arena the arena to allocate the normalized string in
///
/// @return the normalized string
NODISCARD
CT_STD_API char *str_normalize(IN_STRING const char *str, IN_NOTNULL arena_t *arena);

/// @brief turn a string with length into a C string literal
///
/// normalize a string with length into a valid C string literal
///
/// @param text the text to normalize
/// @param arena the arena to allocate the normalized string in
///
/// @return the normalized string
NODISCARD
CT_STD_API char *str_normalizen(text_view_t text, IN_NOTNULL arena_t *arena);

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
NODISCARD
CT_STD_API vector_t *str_split(IN_STRING const char *str, IN_STRING const char *sep, IN_NOTNULL arena_t *arena);

/// @brief find the longest common prefix of a vector of paths
///
/// @note if no common prefix is found, the empty string is returned.
///
/// @param args the vector of paths to find the common prefix of
/// @param arena the arena to allocate the common prefix in
///
/// @return the common prefix
NODISCARD RET_NOTNULL
CT_STD_API const char *str_common_prefix(IN_NOTNULL vector_t *args, IN_NOTNULL arena_t *arena);

/// @brief find the last instance of a substring in a string
///
/// @param str the string to search
/// @param sub the substring to search for
///
/// @return the index of the last instance of @p sub in @p str, or @a SIZE_MAX if
/// sub is not found
RET_INSPECT PUREFN
CT_STD_API size_t str_rfind(IN_STRING const char *str, IN_STRING const char *sub);

/// @brief find the first instance of a substring in a string
///
/// @param str the string to search
/// @param sub the substring to search for
///
/// @return the index of the first instance of @p sub in @p str, or @a SIZE_MAX if @p sub is not found
RET_INSPECT PUREFN
CT_STD_API size_t str_find(IN_STRING const char *str, IN_STRING const char *sub);

/// @brief check if a character is any of a set of characters
///
/// @param c the character to check
/// @param chars the characters to check against
///
/// @retval true @p c is any of @p chars
/// @retval false @p c is not any of @p chars
NODISCARD PUREFN
CT_STD_API bool char_is_any_of(char c, IN_STRING const char *chars);

/// @brief check if a string contains a substring
///
/// @param str the string to search
/// @param search the substring to search for
///
/// @return if @p sub is found in @p str
NODISCARD PUREFN
CT_STD_API bool str_contains(IN_STRING const char *str, IN_STRING const char *search);

/// @brief replace all instances of a substring in a string
///
/// @param str the string to replace elements in
/// @param search the substring to replace
/// @param repl the replacement substring
/// @param arena the arena to allocate the new string in
///
/// @return a copy of @p str with all instances of @p search replaced with @p repl
NODISCARD
CT_STD_API char *str_replace(IN_STRING const char *str, IN_STRING const char *search, IN_STRING const char *repl, IN_NOTNULL arena_t *arena);

/// @brief replace all instances of a each substring in a string with provided replacement
///
/// @param str the string to replace elements in
/// @param repl a map of substrings to replace and their replacements
/// @param arena the arena to allocate the new string in
///
/// @return a copy of @p str with all instances of substrings in @p repl replaced
NODISCARD
CT_STD_API char *str_replace_many(IN_STRING const char *str, IN_NOTNULL const map_t *repl, IN_NOTNULL arena_t *arena);

/// @brief remove all instances of @p letters from @p str
///
/// @param str the string to erase letters from
/// @param len the length of @p str
/// @param letters the letters to erase
///
/// @return @p str with all instances of @p letters removed
NODISCARD NOALIAS
CT_STD_API char *str_erase(IN_READS(len) char *str, size_t len, IN_STRING const char *letters);

/// @brief hash a string
///
/// @param str the string to hash
///
/// @return the hash
NODISCARD PUREFN
CT_STD_API size_t str_hash(IN_STRING const char *str);

/// @brief hash a string with a provided length
///
/// @param text the string to hash
///
/// @return the hash
NODISCARD PUREFN
CT_STD_API size_t text_hash(text_view_t text);

/// @brief compare strings equality
///
/// check if 2 strings are equal
///
/// @param lhs the left hand side of the comparison
/// @param rhs the right hand side of the comparison
///
/// @return if the strings are equal
NODISCARD PUREFN
CT_STD_API bool str_equal(IN_STRING const char *lhs, IN_STRING const char *rhs);

/// @brief get the filename from a path
///
/// @param path the path to get the filename from
/// @param arena the arena to allocate the filename in
///
/// @return the filename extracted from @p path
NODISCARD
CT_STD_API char *str_basename(IN_STRING const char *path, IN_NOTNULL arena_t *arena);

/// @brief get the filename from a path
///
/// @param path the path to get the filename from
/// @param arena the arena to allocate the filename in
///
/// @return the filename extracted from @p path
NODISCARD
CT_STD_API char *str_filename(IN_STRING const char *path, IN_NOTNULL arena_t *arena);

/// @brief remove the last file extension from a path
///
/// @param path the path to remove the extension from
/// @param arena the arena to allocate the path in
///
/// @return the @p path with the last extension removed
NODISCARD
CT_STD_API char *str_noext(IN_STRING const char *path, IN_NOTNULL arena_t *arena);

/// @brief get the last file extension from a path
///
/// @param path the path to get the extension from
/// @param arena the arena to allocate the extension in
///
/// @return the last extension in @p path
NODISCARD
CT_STD_API char *str_ext(IN_STRING const char *path, IN_NOTNULL arena_t *arena);

/// @brief get the directory segment of a path
///
/// @param path the path to get the directory from
/// @param arena the arena to allocate the directory in
///
/// @return the directory extracted from @p path
NODISCARD
CT_STD_API char *str_directory(IN_STRING const char *path, IN_NOTNULL arena_t *arena);

/// @brief uppercase an ascii string
///
/// @param str the string
/// @param arena the arena to allocate the uppercase string in
///
/// @return @p str with all lowercase charaters replaced with uppercase
NODISCARD
CT_STD_API char *str_upper(IN_STRING const char *str, IN_NOTNULL arena_t *arena);

/// @brief lowercase an ascii string
///
/// @param str the string
/// @param arena the arena to allocate the lowercase string in
///
/// @return @p str with all uppercase charaters replaced with lowercase
NODISCARD
CT_STD_API char *str_lower(IN_STRING const char *str, IN_NOTNULL arena_t *arena);

/// @brief get the lowercase version of a character
///
/// @param c the character
///
/// @return the lowercase version of @p c, or the character itself
NODISCARD CONSTFN
CT_STD_API char str_tolower(char c);

/// @brief get the uppercase version of a character
///
/// @param c the character
///
/// @return the uppercase version of @p c or the character itself
NODISCARD CONSTFN
CT_STD_API char str_toupper(char c);

#define STR_WHITESPACE " \t\r\v\n\f" ///< all whitespace charaters

/// @}

END_API
