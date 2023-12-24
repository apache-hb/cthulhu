#include "memory/memory.h"

#include "core/win32.h" // IWYU pragma: keep
#include "os/os.h"
#include "os_common.h"

#include "base/panic.h"

#include "std/str.h"

static BOOL find_next(HANDLE handle, WIN32_FIND_DATA *data, DWORD *error)
{
    BOOL result = FindNextFile(handle, data);
    if (result == 0)
    {
        *error = GetLastError();
    }
    return result;
}

USE_DECL
os_error_t os_iter_begin(const char *path, os_iter_t *result)
{
    CTASSERT(path != NULL);
    CTASSERT(result != NULL);

    char *wild = format("%s" NATIVE_PATH_SEPARATOR "*", path);

    WIN32_FIND_DATA data;
    DWORD error = ERROR_SUCCESS;
    HANDLE find = FindFirstFile(wild, &data);

    if (find == INVALID_HANDLE_VALUE)
    {
        return GetLastError();
    }

    do
    {
        if (!is_special(data.cFileName))
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
void os_iter_end(os_iter_t *iter)
{
    CTASSERT(iter != NULL);

    FindClose(iter->find); // TODO: check result
}

USE_DECL
bool os_iter_next(os_iter_t *iter, os_dir_t *result)
{
    CTASSERT(iter != NULL);
    CTASSERT(result != NULL);

    // check for error from previous iteration
    if (iter->error != ERROR_SUCCESS)
        return false;

    PWIN32_FIND_DATA data = &iter->data;
    os_dir_t dir = { .data = iter->data };

    // get the next directory
    while (find_next(iter->find, data, &iter->error) != 0)
    {
        if (!is_special(data->cFileName))
        {
            break;
        }
    }

    *result = dir;
    return true;
}

USE_DECL
os_error_t os_iter_error(os_iter_t *iter)
{
    CTASSERT(iter != NULL);

    if (iter->error == ERROR_NO_MORE_FILES)
        return ERROR_SUCCESS;

    return iter->error;
}

USE_DECL
const char *os_dir_name(os_dir_t *dir)
{
    CTASSERT(dir != NULL);

    // TODO: does this return the full or relative path?
    // TODO: duplicating this string is not ideal
    return ctu_strdup(dir->data.cFileName);
}
