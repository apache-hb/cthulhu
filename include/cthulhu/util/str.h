#pragma once

#include <stdarg.h>

#include "macros.h"
#include "util.h"
#include "vector.h"

/**
 * @defgroup StringUtils String utility functions
 * @brief string manipulation and processing functions
 * @{
 */

/**
 * @brief format a string
 * 
 * format a string with printf-like syntax
 * 
 * @param fmt the format string
 * @param ... the arguments to format
 * 
 * @return the formatted string
 */
PRINT(1, 2)
NODISCARD RET_STR
char *format(FORMAT_STR const char *fmt, ...) NOTNULL(1);

/**
 * @brief format a string with a @a va_list
 * 
 * format a string with printf-like syntax with a va_list
 * 
 * @param fmt the format string
 * @param args the va_list
 * 
 * @return the formatted string
 */
NODISCARD RET_STR
char *formatv(FORMAT_STR const char *fmt, va_list args) NONULL;

/**
 * @brief see if a string starts with a prefix
 * 
 * check if a string starts with a substring
 * 
 * @param str the string to search
 * @param prefix the prefix to check for
 * 
 * @return if str starts with prefix
 */
NODISCARD
bool str_startswith(IN_STR const char *str, IN_STR const char *prefix) CONSTFN NONULL;

/**
 * check if a string ends with a substring
 * 
 * @param str the string to search
 * @param suffix the suffix to check for
 * 
 * @return if str ends with suffix
 */
NODISCARD
bool str_endswith(IN_STR const char *str, IN_STR const char *suffix) CONSTFN NONULL;

/**
 * @brief join strings 
 * 
 * join a vector of strings together with a separator
 * 
 * @param sep the separator to use
 * @param parts a vector of strings to join
 * 
 * @return the joined string
 */
NODISCARD RET_STR
char *str_join(IN_STR const char *sep, IN_NOTNULL vector_t *parts) NONULL;

/**
 * @brief repeat a string
 * 
 * repeat a string n times
 * 
 * @param str the string to repeat
 * @param times the number of times to repeat
 * 
 * @return the repeated string
 */
NODISCARD RET_STR
char *str_repeat(IN_STR const char *str, size_t times) NOTNULL(1);

/**
 * @brief turn a string into a C string literal
 * 
 * normalize a string into a valid C string
 * 
 * @param str the string to normalize
 * 
 * @return the normalized string
 */
NODISCARD RET_STR
char *str_normalize(IN_STR const char *str) NONULL;

/**
 * @brief turn a string with length into a C string literal
 * 
 * normalize a string with length into a valid C string
 *
 * @param str the string to normalize
 * @param len the length of the string
 * 
 * @return the normalized string
 */
RESULT(strlen(return) == MIN(strlen(str), len))
NODISCARD RET_STR
char *str_normalizen(IN_STR const char *str, size_t len) CONSTFN NONULL;

/**
 * @brief split a string into a vector by a separator
 * 
 * @note the seperator is not included in the resulting substrings.
 * @note if no separator is found, the entire string is returned in the vectors first element.
 * 
 * @param str the string to split
 * @param sep the separator to split by
 * 
 * @return the substrings
 */
ALWAYS(strlen(sep) > 0)
NODISCARD RET_VALID
vector_t *str_split(IN_STR const char *str, IN_STR const char *sep) CONSTFN NONULL;

/**
 * @brief find the longest common prefix of a vector of paths
 * 
 * @note if no common prefix is found, the empty string is returned.
 * 
 * @param args the vector of paths to find the common prefix of
 * 
 * @return the common prefix
 */
ALWAYS(args->used > 0)
NODISCARD RET_STR
const char *common_prefix(vector_t *args) CONSTFN NONULL;

/**
 * @brief find the last instance of a substring in a string
 * 
 * @param str the string to search
 * @param sub the substring to search for
 * 
 * @return the index of the last instance of @a sub in @a str, or SIZE_MAX if sub is not found
 */
ALWAYS(strlen(sub) > 0)
RESULT(return <= strlen(str))
NODISCARD
size_t str_rfind(IN_STR const char *str, IN_STR const char *sub) CONSTFN NONULL;

/**
 * @brief check if a string contains a substring
 * 
 * @param str the string to search
 * @param sub the substring to search for
 * 
 * @return if @a sub is found in @a str
 */ 
ALWAYS(strlen(sub) > 0)
NODISCARD
bool str_contains(IN_STR const char *str, IN_STR const char *sub) CONSTFN NONULL;

/**
 * @brief replace all instances of a substring in a string
 * 
 * @param str the string to replace elements in
 * @param sub the substring to replace
 * @param repl the replacement substring
 * 
 * @return a copy of @a str with all instances of @a sub replaced with @a repl
 */
ALWAYS(strlen(sub) > 0)
NODISCARD RET_STR
char *str_replace(IN_STR const char *str, IN_STR const char *sub, IN_STR const char *repl) NONULL;

/**
 * @brief hash a string
 * 
 * @param str the string to hash
 * 
 * @return the hash
 */
NODISCARD
size_t strhash(IN_STR const char *str) CONSTFN NONULL;

/**
 * @brief compare strings equality
 * 
 * check if 2 strings are equal
 * 
 * @param lhs the left hand side of the comparison
 * @param rhs the right hand side of the comparison
 * 
 * @return if the strings are equal
 */
NODISCARD
bool str_equal(IN_STR const char *lhs, IN_STR const char *rhs) HOT CONSTFN NONULL;

/** @} */

typedef struct {
    size_t len;
    size_t size;
    char *data;
} stream_t;

void stream_delete(stream_t *stream) NONULL;
stream_t *stream_new(size_t size) ALLOC(stream_delete);
size_t stream_len(stream_t *stream) CONSTFN NONULL;
void stream_write(stream_t *stream, const char *str) NONULL;
void stream_write_bytes(stream_t *stream, const void *bytes, size_t len) NONULL;
const char *stream_data(const stream_t *stream) CONSTFN NONULL;
