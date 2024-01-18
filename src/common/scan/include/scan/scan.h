#pragma once

#include <ctu_scan_api.h>

#include "core/analyze.h"
#include "core/compiler.h"
#include "core/text.h"

#include <stddef.h>
#include <stdbool.h>

typedef struct io_t io_t;
typedef struct scan_t scan_t;
typedef struct arena_t arena_t;

BEGIN_API

/// @ingroup location
/// @{

#define SCAN_BUILTIN_LANG "builtin"
/// @def SCAN_BUILTIN_LANG
/// @brief the language of the builtin scanner

#define SCAN_BUILTIN_NAME "<builtin>"
/// @def SCAN_BUILTIN_NAME
/// @brief the name of the builtin scanner

/// @brief get the source language of a scanner
///
/// @param scan the scanner to get the language of
///
/// @return the language of @p scan
NODISCARD PUREFN
CT_SCAN_API const char *scan_language(IN_NOTNULL const scan_t *scan);

/// @brief get the path of a scanner
///
/// @param scan the scanner to get the path of
///
/// @return the path of @p scan
NODISCARD PUREFN
CT_SCAN_API const char *scan_path(IN_NOTNULL const scan_t *scan);

/// @brief get the compiled object from a scanner
///
/// @param scan the scanner to get the user data of
///
/// @return the user data of @p scan
NODISCARD PUREFN
CT_SCAN_API void *scan_get(IN_NOTNULL scan_t *scan);

/// @brief set the compiled object of a scanner
///
/// @param scan the scanner to set the user data of
/// @param value the new user data
CT_SCAN_API void scan_set(IN_NOTNULL scan_t *scan, void *value);

/// @brief get the context of a scanner
///
/// @param scan the scanner to get the context of
/// @param value the new context
CT_SCAN_API void scan_set_context(IN_NOTNULL scan_t *scan, void *value);

/// @brief get the context of a scanner
///
/// @param scan the scanner to get the context of
///
/// @return the context of @p scan
CT_SCAN_API void *scan_get_context(IN_NOTNULL const scan_t *scan);

/// @brief get a text span of the scanners contents
///
/// @param scan the scanner to get the text of
///
/// @return the text of @p scan
NODISCARD PUREFN
CT_SCAN_API text_view_t scan_source(IN_NOTNULL const scan_t *scan);

/// @brief get the arena of a scanner
///
/// @param scan the scanner to get the arena of
///
/// @return the arena of @p scan
NODISCARD PUREFN
CT_SCAN_API arena_t *scan_get_arena(IN_NOTNULL const scan_t *scan);

/// @brief read data from a scanner
///
/// @param scan the scanner to read from
/// @param dst the destination to read into, must be at least @p size bytes
/// @param size the number of bytes to read
///
/// @return the number of bytes read
NODISCARD RET_RANGE(0, size) NOALIAS
CT_SCAN_API size_t scan_read(IN_NOTNULL scan_t *scan, OUT_WRITES(size) void *dst, size_t size);

/// @brief create a scanner from an io source
///
/// @param language the language of the source
/// @param io the io source to use
/// @param arena the allocator to use
///
/// @return the created scanner
NODISCARD
CT_SCAN_API scan_t *scan_io(IN_STRING const char *language,
                IN_NOTNULL io_t *io,
                IN_NOTNULL arena_t *arena);

/// @brief the builtin scanner
CT_SCAN_API extern const scan_t kScanBuiltin;

/// @brief check if a scanner is the builtin scanner
///
/// @param scan the scanner to check
///
/// @return true if @p scan is the builtin scanner
NODISCARD CONSTFN
CT_SCAN_API bool scan_is_builtin(const scan_t *scan);

/// @}

END_API
