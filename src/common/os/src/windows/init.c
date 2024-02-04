#include "arena/arena.h"
#include "os/core.h"

#include "base/util.h"
#include "base/panic.h"

#include <stdlib.h>
#include <crtdbg.h>

#include "core/win32.h" // IWYU pragma: keep
#include "std/str.h"

#if CTU_WIN32_TRICKERY
#   include <windef.h>
#   include <winbase.h>
#   include <pathcch.h>
#endif

void os_init(void)
{
    _CrtSetReportMode(_CRT_ASSERT, 0);
    _set_abort_behavior(0, _WRITE_ABORT_MSG);
}

USE_DECL
os_error_t os_path_parse(const char *path, arena_t *arena, text_t *out)
{
    CTASSERT(path != NULL);
    CTASSERT(arena != NULL);
    CTASSERT(out != NULL);

    DWORD size = GetFullPathNameA(path, 0, NULL, NULL);
    if (size == 0)
    {
        return GetLastError();
    }

    char *buffer = ARENA_MALLOC(size, "path", NULL, arena);

    DWORD result = GetFullPathNameA(path, size, buffer, NULL);
    if (result == 0)
    {
        return GetLastError();
    }

    // replace all forward slashes with backslashes
    for (size_t i = 0; i < result; i++)
    {
        if (buffer[i] == '/')
        {
            buffer[i] = '\\';
        }
    }

    // if there is a UNC path, skip over it before replacing
    // seperators with null terminators
    char *body = buffer;
    if (str_startswith(buffer, "\\\\"))
    {
        char *ptr = buffer + 2;
        while (*ptr != '\\')
        {
            ptr++;
        }

        ptr++;
        body = ptr;
    }

    while (*body)
    {
        if (*body == '\\')
        {
            *body = '\0';
        }

        body++;
    }

    // now create the path
    text_t view = text_make(buffer, result);
    *out = view;

    return 0;
}

USE_DECL
char *os_path_string(const text_t *path, arena_t *arena)
{
    CTASSERT(path != NULL);
    CTASSERT(arena != NULL);

    char *buffer = ARENA_MALLOC(path->length + 1, "path", path, arena);
    // replace all null terminators with backslashes
    for (size_t i = 0; i < path->length; i++)
    {
        if (path->text[i] == '\0')
        {
            buffer[i] = '\\';
        }
        else
        {
            buffer[i] = path->text[i];
        }
    }

    buffer[path->length] = '\0';

    return buffer;
}
