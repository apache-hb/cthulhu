#pragma once

#ifndef I_WILL_BE_INCLUDING_PLATFORM_CODE
#    error "you should not directly include this header"
#endif

#ifdef _WIN32
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
#    define LIBRARY_HANDLE_TYPE HMODULE
#    define FILE_HANDLE_TYPE HANDLE
#    define FILE_SIZE_TYPE LONGLONG
#    define FILE_POS_TYPE DWORD
#    define FILE_READ_TYPE DWORD
#    define FILE_WRITE_TYPE DWORD
#    define cerror_tYPE DWORD
#else
#    include <dlfcn.h>
#    include <stdio.h>
#    define INVALID_LIBRARY_HANDLE NULL
#    define INVALID_FILE_HANDLE NULL
#    define LIBRARY_HANDLE_TYPE void *
#    define FILE_HANDLE_TYPE FILE *
#    define FILE_SIZE_TYPE size_t
#    define FILE_POS_TYPE size_t
#    define FILE_READ_TYPE size_t
#    define FILE_WRITE_TYPE size_t
#    define cerror_tYPE int
#endif

#include "cthulhu/util/macros.h"

#include <stdbool.h>

typedef LIBRARY_HANDLE_TYPE library_handle_t;
typedef FILE_HANDLE_TYPE file_handle_t;
typedef cerror_tYPE native_cerror_t;
typedef FILE_SIZE_TYPE file_size_t;
typedef FILE_POS_TYPE file_pos_t;
typedef FILE_READ_TYPE file_read_t;
typedef FILE_WRITE_TYPE file_write_t;

typedef enum
{
    FORMAT_BINARY,
    FORMAT_TEXT,

    FORMAT_TOTAL
} file_format_t;

typedef enum
{
    MODE_READ,
    MODE_WRITE,

    MODE_TOTAL
} file_mode_t;

library_handle_t native_library_open(const char *path, native_cerror_t *error);
void native_library_close(library_handle_t handle);
void *native_library_get_symbol(library_handle_t handle, const char *symbol, native_cerror_t *error);

file_handle_t native_file_open(const char *path, file_mode_t mode, file_format_t format, native_cerror_t *error);
void native_file_close(file_handle_t handle);

file_read_t native_file_read(file_handle_t handle, void *buffer, file_read_t size, native_cerror_t *error);
file_write_t native_file_write(file_handle_t handle, const void *buffer, file_write_t size, native_cerror_t *error);

file_size_t native_file_size(file_handle_t handle, native_cerror_t *error);

const void *native_file_map(file_handle_t handle, native_cerror_t *error);

char *native_cerror_to_string(native_cerror_t error);
native_cerror_t native_get_last_error(void);
