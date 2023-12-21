#include "common.h"

#include "base/panic.h"

#include "memory/memory.h"
#include "os/os.h"

#include <climits>
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
os_error_t os_file_open(const char *path, os_access_t access, os_file_t **file)
{
    CTASSERT(path != NULL);
    CTASSERT(access & (eAccessRead | eAccessWrite));
    CTASSERT(file != NULL);

    FILE *fd = fopen(path, get_access(access));

    if (fd == NULL)
    {
        return errno;
    }

    arena_t *arena = get_global_arena();
    os_file_t *result = ARENA_MALLOC(arena, sizeof(os_file_t), path, NULL);
    result->file = fd;
    result->path = path;

    *file = result;
    return 0;
}

void os_file_close(os_file_t *file)
{
    CTASSERT(file != NULL);

    fclose(file->file);
}

USE_DECL
os_error_t os_file_read(os_file_t *file, void *buffer, size_t size, size_t *actual)
{
    CTASSERT(file != NULL);
    CTASSERT(buffer != NULL);
    CTASSERT(size > 0);
    CTASSERT(actual != NULL);

    size_t read = fread(buffer, 1, size, file->file);

    if (read < size)
    {
        if (ferror(file->file))
        {
            return errno;
        }
    }

    *actual = read;
    return 0;
}

USE_DECL
os_error_t os_file_write(os_file_t *file, const void *buffer, size_t size, size_t *actual)
{
    CTASSERT(file != NULL);
    CTASSERT(buffer != NULL);
    CTASSERT(size > 0);
    CTASSERT(actual != NULL);

    size_t written = fwrite(buffer, 1, size, file->file);

    if (written < size)
    {
        if (ferror(file->file))
        {
            return errno;
        }
    }

    *actual = written;
    return errno;
}

USE_DECL
os_error_t os_file_size(os_file_t *file, size_t *actual)
{
    CTASSERT(file != NULL);
    CTASSERT(actual != NULL);

    long pos = ftell(file->file);

    if (pos < 0) { return errno; }
    if (fseek(file->file, 0, SEEK_END) < 0) { return errno; }

    long size = ftell(file->file);

    if (size < 0) { return errno; }
    if (fseek(file->file, pos, SEEK_SET) < 0) { return errno; }

    *actual = size;
    return errno;
}

USE_DECL
os_error_t os_file_seek(os_file_t *file, size_t offset, size_t *actual)
{
    CTASSERT(file != NULL);
    CTASSERT(offset < LONG_MAX);
    CTASSERT(actual != NULL);

    int result = fseek(file->file, (long)offset, SEEK_SET);
    if (result < 0)
    {
        return errno;
    }

    *actual = offset;
    return errno;
}

USE_DECL
os_error_t os_file_tell(os_file_t *file, size_t *actual)
{
    CTASSERT(file != NULL);
    CTASSERT(actual != NULL);

    long pos = ftell(file->file);

    if (pos < 0)
    {
        return errno;
    }

    *actual = pos;
    return errno;
}

USE_DECL
os_error_t os_file_map(os_file_t *file, const void **mapped)
{
    CTASSERT(file != NULL);
    CTASSERT(mapped != NULL);

    size_t size = 0;
    os_error_t err = os_file_size(file, &size);
    if (err) { return err; }

    int fd = fileno(file->file);
    void *ptr = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);

    if (ptr == MAP_FAILED)
    {
        return errno;
    }

    *mapped = ptr;
    return 0;
}

USE_DECL
const char *os_file_name(os_file_t *file)
{
    CTASSERT(file != NULL);

    return file->path;
}
