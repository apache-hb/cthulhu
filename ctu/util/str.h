#pragma once

#include <stdarg.h>

#include "util.h"

#define COLOUR_RED "\x1B[1;31m"
#define COLOUR_GREEN "\x1B[1;32m"
#define COLOUR_YELLOW "\x1B[1;33m"
#define COLOUR_BLUE "\x1B[1;34m"
#define COLOUR_PURPLE "\x1B[1;35m"
#define COLOUR_CYAN "\x1B[1;36m"
#define COLOUR_RESET "\x1B[0m"

/**
 * format a string with printf-like syntax
 * 
 * @param fmt the format string
 * @param ... the arguments to format
 * @return the formatted string
 */
PRINT(1, 2)
char *format(const char *fmt, ...);

/**
 * format a string with printf-like syntax with a va_list
 * 
 * @param fmt the format string
 * @param args the va_list
 * @return the formatted string
 */
char *formatv(const char *fmt, va_list args);

/**
 * check if a string starts with a substring
 * 
 * @param str the string to search
 * @param prefix the prefix to check for
 * @return if str starts with prefix
 */
bool startswith(const char *str, const char *prefix);

/**
 * check if a string ends with a substring
 * 
 * @param str the string to search
 * @param suffix the suffix to check for
 * @return if str ends with suffix
 */
bool endswith(const char *str, const char *suffix);

/**
 * join a vector of strings together with a separator
 * 
 * @param sep the separator to use
 * @param parts a vector of strings to join
 * @return the joined string
 */
char *strjoin(const char *sep, vector_t *parts);

/**
 * repeat a string n times
 * 
 * @param str the string to repeat
 * @param times the number of times to repeat
 * 
 * @return the repeated string
 */
char *strmul(const char *str, size_t times);

/**
 * normalize a string into a valid C string
 * 
 * @param str the string to normalize
 * 
 * @return the normalized string
 */
char *strnorm(const char *str);

typedef struct {
    size_t len;
    size_t size;
    char *data;
} stream_t;

stream_t *stream_new(size_t size);
void stream_delete(stream_t *stream);
size_t stream_len(stream_t *stream);
void stream_write(stream_t *stream, const char *str);
const char *stream_data(const stream_t *stream);
