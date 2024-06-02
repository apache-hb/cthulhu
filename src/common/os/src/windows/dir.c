// SPDX-License-Identifier: LGPL-3.0-only
#include "os/os.h"
#include "os_common.h"

#include "core/win32.h" // IWYU pragma: keep

#include "std/str.h"

CT_LOCAL os_iter_impl_t impl_iter_open(const char *path, os_inode_impl_t *inode)
{
    char wildcard[MAX_PATH];
    str_sprintf(wildcard, MAX_PATH, "%s" CT_NATIVE_PATH_SEPARATOR "*", path);

    return FindFirstFileA(wildcard, inode);
}

CT_LOCAL bool impl_iter_next(os_iter_impl_t impl, os_inode_impl_t *inode)
{
    return FindNextFileA(impl, inode) != 0;
}

CT_LOCAL bool impl_iter_close(os_iter_impl_t impl)
{
    return FindClose(impl) != 0;
}

CT_LOCAL const char *impl_inode_name(const os_inode_impl_t *inode)
{
    return inode->cFileName;
}

CT_LOCAL os_dirent_t impl_inode_type(const os_inode_impl_t *inode)
{
    return (inode->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? eOsNodeDir : eOsNodeFile;
}
