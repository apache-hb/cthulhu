#pragma once

#include <ctu_os_api.h>

#include "core/analyze.h"
#include "core/compiler.h"

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
typedef enum os_access_t
{
#define OS_ACCESS(ID, STR, BIT) ID = (BIT),
#include "os.def"
} os_access_t;

/// @brief file mapping memory protection
typedef enum os_protect_t
{
#define OS_PROTECT(ID, STR, BIT) ID = (BIT),
#include "os.def"
} os_protect_t;

/// @brief directory entry type
typedef enum os_dirent_t
{
#define OS_DIRENT(ID, STR) ID,
#include "os.def"

    eOsNodeCount
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

/// @}

CT_END_API

CT_ENUM_FLAGS(os_access_t)
CT_ENUM_FLAGS(os_protect_t)
