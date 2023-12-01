#pragma once

#include "base/analyze.h"
#include <stddef.h>

typedef struct reports_t reports_t;
typedef struct io_t io_t;
typedef struct scan_t scan_t;

/// @brief a span of text inside a scanner
typedef struct text_t
{
    size_t size;      ///< the number of bytes in the text
    const char *text; ///< the text itself
} text_t;

/// @brief get the source language of a scanner
///
/// @param scan the scanner to get the language of
///
/// @return the language of @a scan
NODISCARD CONSTFN
const char *scan_language(IN_NOTNULL const scan_t *scan);

/// @brief get the path of a scanner
///
/// @param scan the scanner to get the path of
///
/// @return the path of @a scan
NODISCARD CONSTFN
const char *scan_path(IN_NOTNULL const scan_t *scan);

/// @brief get the current user data of a scanner
///
/// @param scan the scanner to get the user data of
///
/// @return the user data of @a scan
NODISCARD CONSTFN
void *scan_get(IN_NOTNULL scan_t *scan);

/// @brief set the current user data of a scanner
///
/// @param scan the scanner to set the user data of
/// @param value the new user data
void scan_set(IN_NOTNULL scan_t *scan, void *value);

/// @brief get the current backing text pointer of a scanner
///
/// @param scan the scanner to get the text pointer of
///
/// @return the text pointer of @a scan
NODISCARD CONSTFN
const char *scan_text(IN_NOTNULL const scan_t *scan);

/// @brief get the size of a scanners backing text
///
/// @param scan the scanner to get the text size of
///
/// @return the text size of @a scan
NODISCARD CONSTFN
size_t scan_size(IN_NOTNULL const scan_t *scan);

/// @brief get a text span of the scanners contents
///
/// @param scan the scanner to get the text of
///
/// @return the text of @a scan
NODISCARD CONSTFN
text_t scan_source(IN_NOTNULL const scan_t *scan);

/// @brief get the scanners report sink
///
/// @param scan the scanner to get the report sink of
///
/// @return the report sink of @a scan
NODISCARD CONSTFN
reports_t *scan_reports(IN_NOTNULL scan_t *scan);

/// @brief get the scanners io source
///
/// @param scan the scanner to get the io source of
///
/// @return the io source of @a scan
NODISCARD CONSTFN
io_t *scan_src(IN_NOTNULL scan_t *scan);

/// @brief get an invalid scanner
///
/// @return the invalid scanner
NODISCARD CONSTFN
scan_t *scan_invalid(void);

/// @brief read data from a scanner
///
/// @param scan the scanner to read from
/// @param dst the destination to read into, must be at least @a size bytes
/// @param size the number of bytes to read
///
/// @return the number of bytes read
NODISCARD RET_RANGE(0, size)
size_t scan_read(IN_NOTNULL scan_t *scan, OUT_WRITES(size) void *dst, size_t size);

/// @brief create a scanner from an io source
///
/// @param reports the report sink to use
/// @param language the language of the source
/// @param io the io source to use
///
/// @return the created scanner
NODISCARD
scan_t *scan_io(IN_NOTNULL reports_t *reports, IN_STRING const char *language, IN_NOTNULL io_t *io);
