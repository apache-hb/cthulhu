// SPDX-License-Identifier: LGPL-3.0-only

#include "os/os.h"
#include "os_common.h"

#include "base/panic.h"
#include "base/util.h"

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
os_error_t os_iter_error(const os_iter_t *iter)
{
    CTASSERT(iter != NULL);

    return iter->error;
}

const char *impl_dirname(const os_inode_t *inode)
{
    return inode->ent->d_name;
}
