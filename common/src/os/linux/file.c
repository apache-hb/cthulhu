#include "common.h"

#include "base/panic.h"

#include <stdio.h>
#include <errno.h>

#include <sys/mman.h>

typedef struct os_file_t
{
    const char *path;
    FILE *file;
} os_file_t;

static const char *get_access(os_access_t access)
{
    if (access & eAccessText)
    {
        if (access & eAccessWrite)
        {
            return "w";
        }

        if (access & eAccessRead)
        {
            return "r";
        }
    }

    if (access & eAccessWrite)
    {
        return "wb";
    }

    if (access & eAccessRead)
    {
        return "rb";
    }

    NEVER("invalid access flags 0x%x", access);
}

USE_DECL
OS_RESULT(os_file_t *) os_file_open(const char *path, os_access_t access)
{
    CTASSERT(path != NULL);
    CTASSERT(access & (eAccessRead | eAccessWrite));

    FILE *fd = fopen(path, get_access(access));

    if (fd == NULL)
    {
        return linux_error(errno);
    }

    os_file_t file = {
        .path = path,
        .file = fd
    };

    return os_result_new(0, &file, sizeof(os_file_t));
}

void os_file_close(os_file_t *file)
{
    CTASSERT(file != NULL);

    fclose(file->file);
}

USE_DECL
OS_RESULT(size_t) os_file_read(os_file_t *file, void *buffer, size_t size)
{
    CTASSERT(file != NULL);
    CTASSERT(buffer != NULL);
    CTASSERT(size > 0);

    size_t read = fread(buffer, 1, size, file->file);

    if (read < size)
    {
        if (ferror(file->file))
        {
            return linux_error(errno);
        }
    }

    return linux_result(errno, &read, sizeof(size_t));
}

USE_DECL
OS_RESULT(size_t) os_file_write(os_file_t *file, const void *buffer, size_t size)
{
    CTASSERT(file != NULL);
    CTASSERT(buffer != NULL);
    CTASSERT(size > 0);

    size_t written = fwrite(buffer, 1, size, file->file);

    if (written < size)
    {
        if (ferror(file->file))
        {
            return linux_error(errno);
        }
    }

    return linux_result(errno, &written, sizeof(size_t));
}

USE_DECL
OS_RESULT(size_t) os_file_size(os_file_t *file)
{
    CTASSERT(file != NULL);

    long pos = ftell(file->file);

    if (pos < 0) { return linux_error(errno); }
    if (fseek(file->file, 0, SEEK_END) < 0) { return linux_error(errno); }

    long size = ftell(file->file);

    if (size < 0) { return linux_error(errno); }
    if (fseek(file->file, pos, SEEK_SET) < 0) { return linux_error(errno); }

    size_t result = size;

    return linux_result(errno, &result, sizeof(size_t));
}

USE_DECL
OS_RESULT(size_t) os_file_seek(os_file_t *file, size_t offset)
{
    CTASSERT(file != NULL);

    if (fseek(file->file, offset, SEEK_SET) < 0)
    {
        return linux_error(errno);
    }

    return linux_result(errno, NULL, 0);
}

USE_DECL
OS_RESULT(size_t) os_file_tell(os_file_t *file)
{
    CTASSERT(file != NULL);

    long pos = ftell(file->file);

    if (pos < 0)
    {
        return linux_error(errno);
    }

    size_t result = pos;

    return linux_result(errno, &result, sizeof(size_t));
}

USE_DECL
OS_RESULT(const void *) os_file_map(os_file_t *file)
{
    CTASSERT(file != NULL);

    OS_RESULT(size_t) size = os_file_size(file);
    if (os_error(size)) { return size; }

    int fd = fileno(file->file);
    void *ptr = mmap(NULL, OS_VALUE(size_t, size), PROT_READ, MAP_PRIVATE, fd, 0);

    if (ptr == MAP_FAILED)
    {
        return linux_error(errno);
    }

    return os_result_new(0, &ptr, sizeof(void *));
}

USE_DECL
const char *os_file_name(os_file_t *file)
{
    CTASSERT(file != NULL);

    return file->path;
}
