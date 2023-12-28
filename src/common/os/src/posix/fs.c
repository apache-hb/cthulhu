#include "os/os.h"

#include "base/panic.h"

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

USE_DECL
os_error_t os_file_create(const char *path)
{
    CTASSERT(path != NULL);

    FILE *fd = fopen(path, "wb+");
    if (fd != NULL)
    {
        fclose(fd);
        return 0;
    }

    return errno;
}

USE_DECL
os_error_t os_file_delete(const char *path)
{
    CTASSERT(path != NULL);

    if (remove(path) != 0)
    {
        return errno;
    }

    return errno;
}

USE_DECL
os_error_t os_dir_create(const char *path, bool *create)
{
    CTASSERT(path != NULL);
    CTASSERT(create != NULL);

    // TODO: this is a bit of a hack to avoid a bug in mkdir_recursive
    // on linux it will pass an empty string into mkdir because of the leading `/`
    if (strlen(path) == 0)
    {
        *create = true;
        return 0;
    }

    if (mkdir(path, 0777) != 0)
    {
        if (errno != EEXIST)
        {
            return errno;
        }

        errno = 0;
    }

    *create = (errno == 0);
    return 0;
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
bool os_dir_exists(const char *path)
{
    CTASSERT(path != NULL);

    struct stat sb;
    return stat(path, &sb) == 0 && S_ISDIR(sb.st_mode);
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
os_error_t os_dir_current(char *cwd, size_t size)
{
    CTASSERT(cwd != NULL);
    CTASSERT(size > 0);

    getcwd(cwd, size);
    return errno;
}
