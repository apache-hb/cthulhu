// SPDX-License-Identifier: LGPL-3.0-only

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

/// @brief inode handle
typedef struct os_inode_t os_inode_t;

/// @brief directory iterator
typedef struct os_iter_t os_iter_t;

/// @brief file access mode
typedef enum os_access_t
{
#define OS_ACCESS(ID, STR, BIT) ID = (BIT),
#include "os.inc"
} os_access_t;

/// @brief file mapping memory protection
typedef enum os_protect_t
{
#define OS_PROTECT(ID, STR, BIT) ID = (BIT),
#include "os.inc"
} os_protect_t;

/// @brief directory entry type
typedef enum os_dirent_t
{
#define OS_DIRENT(ID, STR) ID,
#include "os.inc"

    eOsNodeCount
} os_dirent_t;

/// @brief error code
typedef size_t os_error_t;

/// @brief program exit code
typedef int os_exit_t;

/// @brief function pointer
/// used for shared library symbols rather than void*
/// because casting a function pointer to void* is undefined behavior
typedef void (*os_symbol_t)(void);

/// @brief initialize the os api
/// @note this must be called before using any other os api
CT_OS_API void os_init(void);

/// @brief exit the program
///
/// @param code the exit code
CT_NORETURN CT_OS_API os_exit(os_exit_t code);

/// @brief convert an os error code to a string
///
/// @param error the error code to convert
/// @param arena the arena to allocate from
///
/// @return the string representation of the error code
CT_NODISCARD RET_STRING
CT_OS_API char *os_error_string(os_error_t error, IN_NOTNULL arena_t *arena);

/// @brief get the string representation of a directory entry type
///
/// @param type the directory entry type
///
/// @return the string representation
CT_CONSTFN RET_STRING
CT_OS_API const char *os_dirent_string(os_dirent_t type);

/// @brief get the string representation of a file access mode
///
/// @param access the file access mode
///
/// @return the string representation
CT_CONSTFN RET_STRING
CT_OS_API const char *os_access_string(os_access_t access);

/// @brief get the string representation of a file mapping memory protection
///
/// @param protect the file mapping memory protection
///
/// @return the string representation
CT_CONSTFN RET_STRING
CT_OS_API const char *os_protect_string(os_protect_t protect);

/// @}

CT_END_API

CT_ENUM_FLAGS(os_access_t)
CT_ENUM_FLAGS(os_protect_t)
