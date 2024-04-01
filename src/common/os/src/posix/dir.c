// SPDX-License-Identifier: LGPL-3.0-only

#include "os/os.h"
#include "os_common.h"
#include "core/macros.h"

os_iter_impl_t impl_iter_open(const char *path, os_inode_impl_t *inode)
{
    CT_UNUSED(inode);

    return opendir(path);
}

bool impl_iter_next(os_iter_impl_t impl, os_inode_impl_t *inode)
{
    struct dirent *ent = readdir(impl);
    *inode = ent;
    return ent != NULL;
}

bool impl_iter_close(os_iter_impl_t impl)
{
    return closedir(impl) == 0;
}

const char *impl_inode_name(const os_inode_impl_t *inode)
{
    struct dirent *ent = *inode;
    return ent->d_name;
}

os_dirent_t impl_inode_type(const os_inode_impl_t *inode)
{
    struct dirent *ent = *inode;
    return ent->d_type == DT_DIR ? eOsNodeDir : eOsNodeFile;
}
