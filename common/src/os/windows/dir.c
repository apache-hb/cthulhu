#include "common.h"

#include "base/panic.h"

typedef struct os_iter_t
{
    HANDLE find;
    WIN32_FIND_DATA data;
    DWORD error;
} os_iter_t;

typedef struct os_dir_t
{
    WIN32_FIND_DATA data;
} os_dir_t;

OS_RESULT(os_iter_t *) os_iter_begin(const char *path)
{
    CTASSERT(path != NULL);

    WIN32_FIND_DATA data = { 0 };
    HANDLE find = FindFirstFile(path, &data);

    if (find == INVALID_HANDLE_VALUE) 
    { 
        DWORD error = GetLastError();
        switch (error)
        {
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            return NULL;
        default:
            return win_result(error, NULL, 0);
        }
    }

    os_iter_t iter = {
        .find = find,
        .data = data,
        .error = ERROR_SUCCESS
    };

    return win_result(ERROR_SUCCESS, &iter, sizeof(iter));
}

void os_iter_end(os_iter_t *iter)
{
    CTASSERT(iter != NULL);

    FindClose(iter->find); // TODO: check result
}

OS_RESULT(os_dir_t *) os_iter_next(os_iter_t *iter)
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
    while (FindNextFile(iter->find, data) != 0)
    {
        if (data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            break;
        }
    }

    // store error to check on next iteration
    iter->error = GetLastError();

    return win_result(ERROR_SUCCESS, &dir, sizeof(dir));
}

const char *os_dir_name(os_dir_t *dir)
{
    CTASSERT(dir != NULL);

    // TODO: does this return the full or relative path?
    return dir->data.cFileName;
}
