#include "common.h"

#include "base/panic.h"

#include "std/str.h"

#include "report/report.h"

typedef struct os_iter_t
{
    HANDLE hFind;
    WIN32_FIND_DATA data;
    DWORD error;
} os_iter_t;

typedef struct os_dir_t
{
    WIN32_FIND_DATA data;
} os_dir_t;

static BOOL find_next(HANDLE handle, WIN32_FIND_DATA *data, DWORD *error)
{
    BOOL result = FindNextFile(handle, data);
    if (result == 0)
    {
        *error = GetLastError();
    }
    return result;
}

OS_RESULT(os_iter_t) os_iter_begin(const char *path)
{
    CTASSERT(path != NULL);

    char *wild = format("%s" NATIVE_PATH_SEPARATOR "*", path);

    WIN32_FIND_DATA data;
    DWORD error = ERROR_SUCCESS;
    HANDLE find = FindFirstFile(wild, &data);

    if (find == INVALID_HANDLE_VALUE)
    {
        error = GetLastError();
        switch (error)
        {
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            return NULL;
        default:
            return win_result(error, NULL, 0);
        }
    }

    do
    {
        if (!is_special(data.cFileName))
        {
            break;
        }
    } while (find_next(find, &data, &error) != 0);

    os_iter_t iter = {
        .hFind = find,
        .data = data,
        .error = error
    };

    return win_result(ERROR_SUCCESS, &iter, sizeof(os_iter_t));
}

void os_iter_end(os_iter_t *iter)
{
    CTASSERT(iter != NULL);

    FindClose(iter->hFind); // TODO: check result
}

OS_RESULT(os_dir_t) os_iter_next(os_iter_t *iter)
{
    CTASSERT(iter != NULL);

    // check for error from previous iteration
    switch (iter->error)
    {
    case ERROR_SUCCESS: break;
    case ERROR_NO_MORE_FILES: return NULL;
    default: return win_result(iter->error, NULL, 0);
    }

    PWIN32_FIND_DATA data = &iter->data;
    os_dir_t dir = { .data = iter->data };

    // get the next directory
    while (find_next(iter->hFind, data, &iter->error) != 0)
    {
        if (!is_special(data->cFileName))
        {
            break;
        }
    }

    return win_result(ERROR_SUCCESS, &dir, sizeof(os_dir_t));
}

const char *os_dir_name(os_dir_t *dir)
{
    CTASSERT(dir != NULL);

    // TODO: does this return the full or relative path?
    return dir->data.cFileName;
}
