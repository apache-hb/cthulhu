#pragma once

#include "base/compiler.h"

#ifdef OS_WINDOWS
#    define NOMINMAX
#    define WIN32_LEAN_AND_MEAN
#    define NOGDICAPMASKS
#    define NOCRYPT
#    define NOVIRTUALKEYCODES
#    define NOWINMESSAGES
#    define NOWINSTYLES
#    define NOSYSMETRICS
#    define NOMENUS
#    define NOICONS
#    define NOKEYSTATES
#    define NORASTEROPS
#    define NOSYSCOMMANDS
#    define NOSHOWWINDOW
#    define OEMRESOURCE
#    define NOATOM
#    define NOCLIPBOARD
#    define NOCOLOR
#    define NOCTLMGR
#    define NODRAWTEXT
#    define NOGDI
#    define NOMB
#    define NOMEMMGR
#    define NOMETAFILE
#    define NOMSG
#    define NOOPENFILE
#    define NOSCROLL
#    define NOSERVICE
#    define NOSOUND
#    define NOTEXTMETRIC
#    define NOWH
#    define NOWINOFFSETS
#    define NOCOMM
#    define NOKANJI
#    define NOHELP
#    define NOPROFILER
#    define NODEFERWINDOWPOS
#    define NOMCX
#    include <windows.h>

#    define INVALID_LIBRARY_HANDLE NULL
#    define INVALID_FILE_HANDLE INVALID_HANDLE_VALUE
#    define PROCESS_HANDLE_TYPE HANDLE
#    define LIBRARY_HANDLE_TYPE HMODULE
#    define FILE_HANDLE_TYPE HANDLE
#    define FILE_SIZE_TYPE LONGLONG
#    define FILE_POS_TYPE DWORD
#    define FILE_READ_TYPE DWORD
#    define FILE_WRITE_TYPE DWORD
#    define CERROR_TYPE HRESULT
#else
#    include <dlfcn.h>
#    include <stdio.h>
#    define INVALID_LIBRARY_HANDLE NULL
#    define INVALID_FILE_HANDLE NULL
#    define PROCESS_HANDLE_TYPE void *
#    define LIBRARY_HANDLE_TYPE void *
#    define FILE_HANDLE_TYPE FILE *
#    define FILE_SIZE_TYPE size_t
#    define FILE_POS_TYPE size_t
#    define FILE_READ_TYPE size_t
#    define FILE_WRITE_TYPE size_t
#    define CERROR_TYPE int
#endif

#include "base/macros.h"

#include <stdbool.h>

typedef struct vector_t vector_t;

typedef LIBRARY_HANDLE_TYPE library_handle_t;
typedef FILE_HANDLE_TYPE file_handle_t;
typedef PROCESS_HANDLE_TYPE process_handle_t;

typedef CERROR_TYPE native_cerror_t;

typedef FILE_SIZE_TYPE file_size_t;
typedef FILE_POS_TYPE file_pos_t;
typedef FILE_READ_TYPE file_read_t;
typedef FILE_WRITE_TYPE file_write_t;

typedef enum
{
    eFormatBinary,
    eFormatText,

    eFormatTotal
} file_format_t;

typedef enum
{
    eModeRead,
    eModeWrite,

    eModeTotal
} file_mode_t;

/// library api

NODISCARD
library_handle_t native_library_open(const char *path, native_cerror_t *error);
void native_library_close(library_handle_t handle);

NODISCARD
void *native_library_get_symbol(library_handle_t handle, const char *symbol, native_cerror_t *error);

/// file api

NODISCARD
native_cerror_t native_make_directory(const char *path);

void native_delete_directory(const char *path);

NODISCARD
const char *native_get_cwd(void);

NODISCARD
native_cerror_t native_delete_file(const char *path);

NODISCARD
bool native_is_directory(const char *path);

NODISCARD
bool native_is_file(const char *path);

NODISCARD
file_handle_t native_file_open(const char *path, file_mode_t mode, file_format_t format, native_cerror_t *error);
void native_file_close(file_handle_t handle);

file_read_t native_file_read(file_handle_t handle, void *buffer, file_read_t size, native_cerror_t *error);
file_write_t native_file_write(file_handle_t handle, const void *buffer, file_write_t size, native_cerror_t *error);

NODISCARD
file_size_t native_file_size(file_handle_t handle, native_cerror_t *error);

NODISCARD
file_pos_t native_file_seek(file_handle_t handle, file_pos_t offset, native_cerror_t *error);

NODISCARD
const void *native_file_map(file_handle_t handle, native_cerror_t *error);

NODISCARD
char *native_cerror_to_string(native_cerror_t error);

/// error api

void native_platform_init(void);

NODISCARD
native_cerror_t native_get_last_error(void);

NODISCARD
vector_t *native_error_stacktrace(void);
