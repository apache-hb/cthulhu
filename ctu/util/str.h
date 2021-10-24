#pragma once

#include <stdarg.h>

#include "macros.h"
#include "util.h"

/**
 * format a string with printf-like syntax
 * 
 * @param fmt the format string
 * @param ... the arguments to format
 * @return the formatted string
 */
PRINT(1, 2)
char *format(const char *fmt, ...) NOTNULL(1);

/**
 * format a string with printf-like syntax with a va_list
 * 
 * @param fmt the format string
 * @param args the va_list
 * @return the formatted string
 */
char *formatv(const char *fmt, va_list args) NONULL;

/**
 * check if a string starts with a substring
 * 
 * @param str the string to search
 * @param prefix the prefix to check for
 * @return if str starts with prefix
 */
bool startswith(const char *str, const char *prefix) CONSTFN NONULL;

/**
 * check if a string ends with a substring
 * 
 * @param str the string to search
 * @param suffix the suffix to check for
 * @return if str ends with suffix
 */
bool endswith(const char *str, const char *suffix) CONSTFN NONULL;

/**
 * join a vector of strings together with a separator
 * 
 * @param sep the separator to use
 * @param parts a vector of strings to join
 * @return the joined string
 */
OWNED char *strjoin(const char *sep, WEAK vector_t *parts) NONULL;

/**
 * repeat a string n times
 * 
 * @param str the string to repeat
 * @param times the number of times to repeat
 * 
 * @return the repeated string
 */
char *strmul(const char *str, size_t times) NOTNULL(1);

/**
 * normalize a string into a valid C string
 * 
 * @param str the string to normalize
 * 
 * @return the normalized string
 */
char *strnorm(const char *str) NONULL;

/**
 * normalize a string with length into a valid C string
 *
 * @param str the string to normalize
 * @param len the length of the string
 * 
 * @return the normalized string
 */
char *nstrnorm(const char *str, size_t len) CONSTFN NONULL;

char *strslice(const char *str, size_t start, size_t end) CONSTFN NONULL;

vector_t *strsplit(const char *str, const char *sep) CONSTFN NONULL;

const char *common_prefix(vector_t *strings) CONSTFN NONULL;

size_t rfind(const char *str, char c) CONSTFN NONULL;

/**
 * hash a string into a size_t
 * 
 * @param str the string to hash
 * 
 * @return the hash
 */
size_t strhash(const char *str) CONSTFN NONULL;

/**
 * check if 2 strings are equal
 * 
 * @param lhs the left hand side of the comparison
 * @param rhs the right hand side of the comparison
 * 
 * @return if the strings are equal
 */
bool streq(const char *lhs, const char *rhs) HOT CONSTFN NONULL;

typedef struct {
    size_t len;
    size_t size;
    OWNED char *data;
} stream_t;

void stream_delete(stream_t *stream) NONULL;
stream_t *stream_new(size_t size) ALLOC(stream_delete);
size_t stream_len(stream_t *stream) CONSTFN NONULL;
void stream_write(stream_t *stream, const char *str) NONULL;
const char *stream_data(const stream_t *stream) CONSTFN NONULL;
