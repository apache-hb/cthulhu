#include "common.h"

#include "base/panic.h"

#include "report/report.h"

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

USE_DECL
OS_RESULT(bool) os_file_create(const char *path)
{
    CTASSERT(path != NULL);

    FILE *fd = fopen(path, "wb+");
    if (fd == NULL)
    {
        return linux_error(errno);
    }

    fclose(fd);

    bool created = (errno == 0);
    return linux_result(0, &created, sizeof(bool));
}

USE_DECL
OS_RESULT(bool) os_file_delete(const char *path)
{
    CTASSERT(path != NULL);

    if (remove(path) != 0)
    {
        return linux_error(errno);
    }

    bool removed = (errno == 0);
    return linux_result(errno, &removed, sizeof(bool));
}

USE_DECL
OS_RESULT(bool) os_file_exists(const char *path)
{
    CTASSERT(path != NULL);

    bool exists = access(path, F_OK) == 0;
    return linux_result(errno, &exists, sizeof(bool));
}

USE_DECL
OS_RESULT(bool) os_dir_create(const char *path)
{
    CTASSERT(path != NULL);

    logverbose("os_dir_create: %s", path);

    if (mkdir(path, 0777) != 0)
    {
        if (errno != EEXIST)
        {
            return linux_error(errno);
        }

        errno = 0;
    }

    bool created = (errno == 0);
    return linux_result(0, &created, sizeof(bool));
}

USE_DECL
OS_RESULT(bool) os_dir_delete(const char *path)
{
    CTASSERT(path != NULL);

    if (rmdir(path) != 0)
    {
        return linux_error(errno);
    }

    bool removed = (errno == 0);
    return linux_result(errno, &removed, sizeof(bool));
}

USE_DECL
OS_RESULT(bool) os_dir_exists(const char *path)
{
    CTASSERT(path != NULL);

    struct stat sb;
    bool exists = stat(path, &sb) == 0 && S_ISDIR(sb.st_mode);
    return linux_result(errno, &exists, sizeof(bool));
}

USE_DECL
OS_RESULT(bool) os_dirent_exists(const char *path)
{
    CTASSERT(path != NULL);

    struct stat sb;
    bool exists = stat(path, &sb) == 0;
    return linux_result(errno, &exists, sizeof(bool));
}

USE_DECL
OS_RESULT(os_dirent_t) os_dirent_type(const char *path)
{
    CTASSERT(path != NULL);

    struct stat sb;
    if (stat(path, &sb) != 0)
    {
        if (errno == ENOENT)
        {
            os_dirent_t ent = eOsNodeNone;
            return os_result_new(0, &ent, sizeof(os_dirent_t));
        }

        return linux_error(errno);
    }

    if (sb.st_mode & S_IFDIR) 
    {
        os_dirent_t ent = eOsNodeDir;
        return os_result_new(0, &ent, sizeof(os_dirent_t));
    }
    else if (sb.st_mode & S_IFREG)
    {
        os_dirent_t ent = eOsNodeFile;
        return os_result_new(0, &ent, sizeof(os_dirent_t));
    }

    os_dirent_t ent = eOsNodeNone;
    return os_result_new(0, &ent, sizeof(os_dirent_t));
}

USE_DECL
OS_RESULT(const char *) os_dir_current(void)
{
    char *path = getcwd(NULL, 0);
    if (path == NULL)
    {
        return linux_error(errno);
    }

    return os_result_new(0, &path, sizeof(char *));
}
