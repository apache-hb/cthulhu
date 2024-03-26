// SPDX-License-Identifier: LGPL-3.0-only

#include "os/os.h"
#include "os_common.h"

#include "base/util.h"
#include "core/win32.h" // IWYU pragma: keep
#include "arena/arena.h"

#include "base/panic.h"

#include "std/str.h"

#if CTU_WIN32_TRICKERY
#   include <winbase.h>
#endif

static BOOL find_next(HANDLE handle, WIN32_FIND_DATA *data, DWORD *error)
{
    BOOL result = FindNextFileA(handle, data);
    if (result == 0)
    {
        *error = GetLastError();
    }
    return result;
}

USE_DECL
os_error_t os_iter_begin(const char *path, os_iter_t *result, arena_t *arena)
{
    CTASSERT(path != NULL);
    CTASSERT(result != NULL);

    char *wild = str_format(arena, "%s" CT_NATIVE_PATH_SEPARATOR "*", path);

    WIN32_FIND_DATA data;
    DWORD error = ERROR_SUCCESS;
    HANDLE find = FindFirstFileA(wild, &data);

    if (find == INVALID_HANDLE_VALUE)
    {
        return GetLastError();
    }

    do
    {
        if (!is_path_special(data.cFileName))
        {
            break;
        }
    } while (find_next(find, &data, &error) != 0);

    os_iter_t iter = {
        .find = find,
        .data = data,
        .error = error
    };

    *result = iter;
    return ERROR_SUCCESS;
}

USE_DECL
os_error_t os_iter_end(os_iter_t *iter)
{
    CTASSERT(iter != NULL);

    if (FindClose(iter->find) == 0)
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}

USE_DECL
bool os_iter_next(os_iter_t *iter, os_inode_t *result)
{
    CTASSERT(iter != NULL);
    CTASSERT(result != NULL);

    // check for error from previous iteration
    if (iter->error != ERROR_SUCCESS)
        return false;

    PWIN32_FIND_DATA data = &iter->data;
    os_inode_t dir = { .data = iter->data };

    // get the next directory
    while (find_next(iter->find, data, &iter->error) != 0)
    {
        if (!is_path_special(data->cFileName))
        {
            break;
        }
    }

    *result = dir;
    return true;
}

USE_DECL
os_error_t os_iter_error(const os_iter_t *iter)
{
    CTASSERT(iter != NULL);

    if (iter->error == ERROR_NO_MORE_FILES)
        return ERROR_SUCCESS;

    return iter->error;
}

const char *impl_dirname(const os_inode_t *inode)
{
    return inode->data.cFileName;
}
