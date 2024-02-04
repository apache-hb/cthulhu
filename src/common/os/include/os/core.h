#pragma once

#include <ctu_os_api.h>

#include "core/analyze.h"
#include "core/compiler.h"
#include "core/text.h"

#include <stddef.h>

typedef struct path_t path_t;
typedef struct arena_t arena_t;

CT_BEGIN_API

/// @ingroup os
/// @{

/// @brief file handle
typedef struct os_file_t os_file_t;

/// @brief directory handle
typedef struct os_dir_t os_dir_t;

/// @brief directory iterator
typedef struct os_iter_t os_iter_t;

/// @brief file access mode
typedef enum os_mode_t
{
    eAccessRead = (1 << 0),  ///< file is readable
    eAccessWrite = (1 << 1), ///< file is writeable, does not imply readable
} os_access_t;

/// @brief file mapping memory protection
typedef enum os_protect_t
{
    eProtectNone = 0, ///< memory cannot be accessed

    eProtectRead = (1 << 0), ///< memory can be read
    eProtectWrite = (1 << 1), ///< memory can be written to (does not imply read)
    eProtectExecute = (1 << 2), ///< memory can be executed (does not imply read or write)
} os_protect_t;

/// @brief directory entry type
typedef enum os_dirent_t
{
    eOsNodeNone, ///< unknown node type
    eOsNodeFile, ///< file node type
    eOsNodeDir,  ///< directory node type
    eOsNodeError,///< error node type

    eOsNodeTotal
} os_dirent_t;

/// @brief error code
typedef size_t os_error_t;

/// @brief function pointer
/// used for shared library symbols rather than void*
/// because casting a function pointer to void* is undefined behavior
typedef void (*os_fn_t)(void);

/// @brief initialize the os api
/// @note this must be called before using any other os api
CT_OS_API void os_init(void);

/// @brief convert an os error code to a string
///
/// @param error the error code to convert
/// @param arena the arena to allocate from
///
/// @return the string representation of the error code
CT_NODISCARD RET_STRING
CT_OS_API char *os_error_string(os_error_t error, IN_NOTNULL arena_t *arena);

/// @brief parse a string into a path
/// parses a platform specific path string into a generic path object
///
/// @param path the path to parse
/// @param arena the arena to allocate from
///
/// @return the parsed path
CT_NODISCARD
CT_OS_API os_error_t os_path_parse(IN_STRING const char *path, IN_NOTNULL arena_t *arena, text_t *out);

/// @brief get the string representation of a path for the current platform
///
/// @param path the path to get the string representation of
/// @param arena the arena to allocate from
///
/// @return the string representation of the path
CT_NODISCARD
CT_OS_API char *os_path_string(IN_NOTNULL const text_t *path, IN_NOTNULL arena_t *arena);

/// @}

CT_END_API
