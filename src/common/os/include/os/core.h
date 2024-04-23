// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_os_api.h>

#include "core/analyze.h"

#if CT_STA_PRESENT
#   include "os/impl/impl.h"
#endif

#include <stddef.h>

typedef struct path_t path_t;
typedef struct arena_t arena_t;
typedef struct text_t text_t;

CT_BEGIN_API

/// @ingroup os
/// @{

/// @brief file handle
typedef struct os_file_t os_file_t;

/// @brief memory mapping handle
typedef struct os_mapping_t os_mapping_t;

/// @brief inode handle
typedef struct os_inode_t os_inode_t;

/// @brief directory iterator
typedef struct os_iter_t os_iter_t;

/// @brief file access mode
typedef enum os_access_t
{
#define OS_ACCESS(ID, STR, BIT) ID = (BIT),
#include "os.inc"

    eOsAccessMask = eOsAccessRead | eOsAccessWrite | eOsAccessTruncate,
} os_access_t;

/// @brief file mapping memory protection
typedef enum os_protect_t
{
#define OS_PROTECT(ID, STR, BIT) ID = (BIT),
#include "os.inc"

    eOsProtectMask = eOsProtectRead | eOsProtectWrite | eOsProtectExecute,
} os_protect_t;

/// @brief directory entry type
typedef enum os_dirent_t
{
#define OS_DIRENT(ID, STR) ID,
#include "os.inc"

    eOsNodeCount
} os_dirent_t;

/// @brief error code
typedef STA_SUCCESS_TYPE(return == eOsSuccess) size_t os_error_t;

/// @brief program exit code
typedef int os_exitcode_t;

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
CT_NORETURN CT_OS_API os_exit(os_exitcode_t code);

/// @brief abort the program
CT_NORETURN CT_OS_API os_abort(void);

/// @brief convert an os error code to a string
/// writes to a buffer rather than allocating.
/// if @p size is 0, the function will return the number of characters needed to write the string.
///
/// @pre @p buffer must point to a valid buffer of at least @p size chars
///
/// @param error the error code to convert
/// @param buffer the buffer to write to
/// @param size the size of the buffer
///
/// @return the number of characters written
CT_NODISCARD
CT_OS_API size_t os_error_get_string(os_error_t error, OUT_WRITES(size) char *buffer, size_t size);

/// @brief get the current working directory
///
/// @pre @p buffer must point to a valid buffer of at least @p size chars
///
/// @param buffer the buffer to write to
/// @param size the size of the buffer
///
/// @return the number of characters written, or 0 on error
CT_NODISCARD
CT_OS_API size_t os_cwd_get_string(OUT_WRITES(size) char *buffer, size_t size);

/// @brief convert an os error code to a string
///
/// @param error the error code to convert
/// @param arena the arena to allocate from
///
/// @return the string representation of the error code
CT_NODISCARD RET_STRING
CT_OS_API char *os_error_string(os_error_t error, IN_NOTNULL arena_t *arena);

/// @brief get the current working directory
///
/// @param arena the arena to allocate from
///
/// @return an error if the current working directory could not be retrieved
RET_INSPECT
CT_OS_API char *os_cwd_string(IN_NOTNULL arena_t *arena);

/// @brief get the current working directory
///
/// @param text the text to write to
/// @param arena the arena to allocate from
///
/// @return an error if the current working directory could not be retrieved
RET_INSPECT
CT_OS_API os_error_t os_getcwd(OUT_NOTNULL text_t *text, IN_NOTNULL arena_t *arena);

/// @brief get the type of an inode
///
/// @param node the inode to get the type of
///
/// @return the type of the inode
CT_NODISCARD
CT_OS_API os_dirent_t os_inode_type(IN_NOTNULL const os_inode_t *node);

/// @brief get the name of an inode
/// @note the name is only valid for the lifetime of the inode
///
/// @param node the inode to get the name of
///
/// @return the name of the inode
CT_NODISCARD
CT_OS_API const char *os_inode_name(IN_NOTNULL const os_inode_t *node);

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

CT_ENUM_FLAGS(os_access_t, int)
CT_ENUM_FLAGS(os_protect_t, int)
