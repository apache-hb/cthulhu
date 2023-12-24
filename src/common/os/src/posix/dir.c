#include "common.h"

#include "base/panic.h"

#include "memory/memory.h"

#include <errno.h>

USE_DECL
os_error_t os_iter_begin(const char *path, os_iter_t *result)
{
    CTASSERT(path != NULL);
    CTASSERT(result != NULL);

    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        return errno;
    }

    os_iter_t iter = {
        .dir = dir
    };
    *result = iter;
    return 0;
}

void os_iter_end(os_iter_t *iter)
{
    CTASSERT(iter != NULL);

    closedir(iter->dir);
}

USE_DECL
bool os_iter_next(os_iter_t *iter, os_dir_t *result)
{
    CTASSERT(iter != NULL);
    CTASSERT(result != NULL);

    struct dirent *ent = NULL;
    while ((ent = readdir(iter->dir)) != NULL)
    {
        if (!is_special(ent->d_name))
        {
            break;
        }
    }

    if (ent == NULL)
    {
        iter->error = errno;
        return false;
    }

    os_dir_t dir = {
        .ent = ent
    };
    *result = dir;

    return true;
}

USE_DECL
os_error_t os_iter_error(os_iter_t *iter)
{
    CTASSERT(iter != NULL);

    return iter->error;
}

USE_DECL
const char *os_dir_name(os_dir_t *dir)
{
    CTASSERT(dir != NULL);

    // have to copy the string because it's owned by the dirent
    return ctu_strdup(dir->ent->d_name);
}
