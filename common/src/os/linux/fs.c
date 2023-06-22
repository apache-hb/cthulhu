#include "common.h"

#include "base/panic.h"

#include <dirent.h>
#include <errno.h>

typedef struct os_iter_t
{
    DIR *dir;
} os_iter_t;

typedef struct os_dir_t
{
    struct dirent *ent;
} os_dir_t;

USE_DECL
OS_RESULT(os_iter_t *) os_iter_begin(const char *path)
{
    CTASSERT(path != NULL);

    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        return linux_error(errno);
    }

    os_iter_t iter = {
        .dir = dir
    };

    return os_result_new(0, &iter, sizeof(os_iter_t));
}

void os_iter_end(os_iter_t *iter)
{
    CTASSERT(iter != NULL);

    closedir(iter->dir);
}

USE_DECL
OS_RESULT(os_dir_t *) os_iter_next(os_iter_t *iter)
{
    CTASSERT(iter != NULL);

    struct dirent *ent = readdir(iter->dir);
    if (ent == NULL)
    {
        if (errno != 0)
        {
            return linux_error(errno);
        }
        else
        {
            return NULL;
        }
    }

    os_dir_t dir = {
        .ent = ent
    };

    return os_result_new(0, &dir, sizeof(os_dir_t));
}

USE_DECL
const char *os_dir_name(os_dir_t *dir)
{
    CTASSERT(dir != NULL);

    return dir->ent->d_name;
}
