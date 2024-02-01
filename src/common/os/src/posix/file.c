#include "os/os.h"

#include "base/panic.h"

#include <limits.h>
#include <stdio.h>
#include <errno.h>

#include <sys/mman.h>

USE_DECL
os_error_t os_file_copy(const char *src, const char *dst)
{
    CTASSERT(src != NULL);
    CTASSERT(dst != NULL);

    // do naive copy for now
    // theres a pile of platform specific calls to do this faster
    // copyfile on macos, sendfile on linux, copy_file_range on bsd, etc

    FILE *src_file = fopen(src, "rb");
    if (src_file == NULL)
    {
        return errno;
    }

    FILE *dst_file = fopen(dst, "wb");
    if (dst_file == NULL)
    {
        fclose(src_file); // TODO: duplicated logic with cleanup
        return errno;
    }

    char buffer[4096];
    size_t read = 0;
    size_t written = 0;

    int result = 0;

    while ((read = fread(buffer, 1, sizeof(buffer), src_file)) > 0)
    {
        written = fwrite(buffer, 1, read, dst_file);

        if (written < read)
        {
            if (ferror(dst_file))
            {
                result = errno;
                break;
            }
        }
    }

    fclose(src_file);
    fclose(dst_file);
    return result;
}

static const char *get_access(os_access_t access)
{
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
os_error_t os_file_exists(const char *path, bool *exists)
{
    CTASSERT(path != NULL);
    CTASSERT(exists != NULL);

    if (access(path, F_OK) == 0)
    {
        *exists = true;
        return 0;
    }

    if (errno == ENOENT)
    {
        *exists = false;
        return 0;
    }

    return errno;
}

USE_DECL
os_error_t os_file_open(const char *path, os_access_t access, os_file_t *file)
{
    CTASSERT(path != NULL);
    CTASSERT(access & (eAccessRead | eAccessWrite));
    CTASSERT(file != NULL);

    FILE *fd = fopen(path, get_access(access));

    if (fd == NULL)
    {
        return errno;
    }

    os_file_t result = {
        .path = path,
        .file = fd,
    };

    *file = result;
    return 0;
}

void os_file_close(os_file_t *file)
{
    CTASSERT(file != NULL);

    // TODO: check result
    (void)fclose(file->file);
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
os_error_t os_file_expand(os_file_t *file, size_t size)
{
    CTASSERT(file != NULL);

    // save the current position
    long pos = ftell(file->file);
    if (pos < 0)
        return errno;

    int result = ftruncate(fileno(file->file), size);
    if (result < 0)
        return errno;

    // restore the position
    result = fseek(file->file, pos, SEEK_SET);
    if (result < 0)
        return errno;

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

static int get_mmap_prot(os_protect_t protect)
{
    // TODO: i hope prot_none is 0
    int result = PROT_NONE;

    if (protect & eProtectRead)
    {
        result |= PROT_READ;
    }

    if (protect & eProtectWrite)
    {
        result |= PROT_WRITE;
    }

    if (protect & eProtectExecute)
    {
        result |= PROT_EXEC;
    }

    return result;
}

USE_DECL
os_error_t os_file_map(os_file_t *file, os_protect_t protect, size_t size, os_mapping_t *mapping)
{
    CTASSERT(file != NULL);
    CTASSERT(mapping != NULL);

    int prot = get_mmap_prot(protect);

    int fd = fileno(file->file);
    void *ptr = mmap(NULL, size, prot, MAP_PRIVATE, fd, 0);

    os_mapping_t result = {
        .data = ptr,
        .size = size,
    };

    *mapping = result;

    // always store the result of mmap in the mapping so we can check for MAP_FAILED
    return (ptr == MAP_FAILED) ? errno : 0;
}

USE_DECL
void os_file_unmap(os_mapping_t *mapping)
{
    CTASSERT(mapping != NULL);

    munmap(mapping->data, mapping->size);
}

USE_DECL
void *os_mapping_data(os_mapping_t *mapping)
{
    CTASSERT(mapping != NULL);

    return mapping->data;
}

USE_DECL
bool os_mapping_active(os_mapping_t *mapping)
{
    CTASSERT(mapping != NULL);

    // if mmap ever gives us 0 as a valid address we'll need to change this
    return mapping->data != NULL
        && mapping->data != MAP_FAILED;
}

USE_DECL
const char *os_file_name(os_file_t *file)
{
    CTASSERT(file != NULL);

    return file->path;
}
