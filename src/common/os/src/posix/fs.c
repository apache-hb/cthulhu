// SPDX-License-Identifier: LGPL-3.0-only

#include "os/os.h"
#include "os_common.h"

#include "base/panic.h"
#include "base/util.h"

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

USE_DECL
os_error_t os_file_delete(const char *path)
{
    CTASSERT(path != NULL);

    if (remove(path) != 0)
    {
        return errno;
    }

    return 0;
}

USE_DECL
os_error_t os_dir_create(const char *path)
{
    CTASSERT(path != NULL);

    // TODO: this is a bit of a hack to avoid a bug in mkdir_recursive
    // on linux it will pass an empty string into mkdir because of the leading `/`
    if (ctu_strlen(path) == 0)
    {
        return eOsSuccess;
    }

    if (mkdir(path, 0777) != 0)
    {
        if (errno == EEXIST)
        {
            return eOsExists;
        }
    }

    return errno;
}

USE_DECL
os_error_t os_dir_delete(const char *path)
{
    CTASSERT(path != NULL);

    if (rmdir(path) != 0)
    {
        return errno;
    }

    return errno;
}

USE_DECL
os_dirent_t os_dirent_type(const char *path)
{
    CTASSERT(path != NULL);

    struct stat sb;
    if (stat(path, &sb) != 0)
    {
        if (errno == ENOENT)
        {
            return eOsNodeNone;
        }

        return eOsNodeError;
    }

    if (sb.st_mode & S_IFDIR)
    {
        return eOsNodeDir;
    }
    else if (sb.st_mode & S_IFREG)
    {
        return eOsNodeFile;
    }

    return eOsNodeNone;
}

USE_DECL
size_t os_cwd_get_string(char *buffer, size_t size)
{
    if (size == 0)
    {
        // caller is asking for the size of the buffer
        CTASSERT(buffer == NULL);
        return impl_maxname();
    }

    CTASSERT(buffer != NULL);

    if (getcwd(buffer, size) == NULL)
    {
        return 0;
    }

    return strlen(buffer);
}
