#pragma once

#include "core/analyze.h"
#include "core/compiler.h"
#include "core/text.h"

#include <stddef.h>
#include <stdbool.h>

typedef struct reports_t reports_t;
typedef struct io_t io_t;
typedef struct scan_t scan_t;
typedef struct arena_t arena_t;

BEGIN_API

/// @ingroup Location
/// @{

/// @brief get the source language of a scanner
///
/// @param scan the scanner to get the language of
///
/// @return the language of @p scan
NODISCARD CONSTFN
const char *scan_language(IN_NOTNULL const scan_t *scan);

/// @brief get the path of a scanner
///
/// @param scan the scanner to get the path of
///
/// @return the path of @p scan
NODISCARD CONSTFN
const char *scan_path(IN_NOTNULL const scan_t *scan);

/// @brief get the current user data of a scanner
///
/// @param scan the scanner to get the user data of
///
/// @return the user data of @p scan
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
/// @return the text pointer of @p scan
NODISCARD CONSTFN
const char *scan_text(IN_NOTNULL const scan_t *scan);

/// @brief get the size of a scanners backing text
///
/// @param scan the scanner to get the text size of
///
/// @return the text size of @p scan
NODISCARD CONSTFN
size_t scan_size(IN_NOTNULL const scan_t *scan);

/// @brief get a text span of the scanners contents
///
/// @param scan the scanner to get the text of
///
/// @return the text of @p scan
NODISCARD CONSTFN
text_view_t scan_source(IN_NOTNULL const scan_t *scan);

/// @brief get the scanners report sink
///
/// @param scan the scanner to get the report sink of
///
/// @return the report sink of @p scan
NODISCARD CONSTFN
reports_t *scan_reports(IN_NOTNULL scan_t *scan);

/// @brief get the scanners io source
///
/// @param scan the scanner to get the io source of
///
/// @return the io source of @p scan
NODISCARD CONSTFN
io_t *scan_src(IN_NOTNULL scan_t *scan);

NODISCARD CONSTFN
arena_t *scan_alloc(IN_NOTNULL const scan_t *scan);

/// @brief get an invalid scanner
///
/// @return the invalid scanner
NODISCARD CONSTFN
scan_t *scan_invalid(void);

/// @brief read data from a scanner
///
/// @param scan the scanner to read from
/// @param dst the destination to read into, must be at least @p size bytes
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
/// @param alloc the allocator to use
///
/// @return the created scanner
NODISCARD
scan_t *scan_io(
    IN_NOTNULL reports_t *reports,
    IN_STRING const char *language,
    IN_NOTNULL io_t *io,
    IN_NOTNULL arena_t *alloc);

NODISCARD CONSTFN
scan_t *scan_builtin(void);

NODISCARD CONSTFN
bool scan_is_builtin(const scan_t *scan);

/// @}

END_API
