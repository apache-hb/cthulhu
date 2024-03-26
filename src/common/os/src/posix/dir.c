// SPDX-License-Identifier: LGPL-3.0-only

#include "os/os.h"

#include "arena/arena.h"

#include "base/panic.h"
#include "base/util.h"

#include "core/macros.h"

#include <errno.h>

USE_DECL
os_error_t os_iter_begin(const char *path, os_iter_t *result, arena_t *arena)
{
    CTASSERT(path != NULL);
    CTASSERT(result != NULL);

    CT_UNUSED(arena);

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

os_error_t os_iter_end(os_iter_t *iter)
{
    CTASSERT(iter != NULL);

    if (closedir(iter->dir) != 0)
    {
        return errno;
    }

    return 0;
}

USE_DECL
bool os_iter_next(os_iter_t *iter, os_inode_t *result)
{
    CTASSERT(iter != NULL);
    CTASSERT(result != NULL);

    struct dirent *ent = NULL;
    while ((ent = readdir(iter->dir)) != NULL)
    {
        if (!is_path_special(ent->d_name))
        {
            break;
        }
    }

    if (ent == NULL)
    {
        iter->error = errno;
        return false;
    }

    os_inode_t dir = {
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
size_t os_dir_get_string(const os_inode_t *dir, char *buffer, size_t size)
{
    CTASSERT(dir != NULL);

    if (size == 0)
    {
        return NAME_MAX;
    }

    CTASSERT(buffer != NULL);

    size_t len = ctu_strlen(dir->ent->d_name);
    size_t ret = CT_MIN(size, len);
    ctu_strcpy(buffer, dir->ent->d_name, ret);

    return ret;
}
