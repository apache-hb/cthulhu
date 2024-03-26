// SPDX-License-Identifier: LGPL-3.0-only

#include "os/os.h"

#include "base/panic.h"

#include <limits.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <sys/mman.h>

static const char *get_access(os_access_t access)
{
    switch (access)
    {
    case eOsAccessRead:
        return "rb";

    case eOsAccessWrite:
    case (eOsAccessRead | eOsAccessWrite):
        return "w+b";

    case (eOsAccessWrite | eOsAccessTruncate):
    case (eOsAccessRead | eOsAccessWrite | eOsAccessTruncate):
        return "wb";

    default:
        CT_NEVER("invalid access flags %s", os_access_string(access));
    }
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
    CTASSERT(file != NULL);
    CTASSERTF(access & (eOsAccessRead | eOsAccessWrite), "%s: invalid access flags 0x%x", path, access);
    CTASSERTF(access != (eOsAccessRead | eOsAccessTruncate), "%s: cannot truncate read only file", path);

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

USE_DECL
os_error_t os_tmpfile_open(os_file_t *file)
{
    CTASSERT(file != NULL);

    FILE *fd = tmpfile();

    if (fd == NULL)
    {
        return errno;
    }

    os_file_t result = {
        .path = "<tmpfile>",
        .file = fd,
    };

    *file = result;
    return 0;
}

os_error_t os_file_close(os_file_t *file)
{
    CTASSERT(file != NULL);

    if (fclose(file->file) != 0)
    {
        return errno;
    }

    return 0;
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
    return 0;
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
    int result = PROT_NONE;

    if (protect & eOsProtectRead)
    {
        result |= PROT_READ;
    }

    if (protect & eOsProtectWrite)
    {
        result |= PROT_WRITE;
    }

    if (protect & eOsProtectExecute)
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
        .view = ptr,
        .size = size,
    };

    *mapping = result;

    // always store the result of mmap in the mapping so we can check for MAP_FAILED
    return (ptr == MAP_FAILED) ? errno : 0;
}

USE_DECL
os_error_t os_file_unmap(os_mapping_t *mapping)
{
    CTASSERT(mapping != NULL);

    if (munmap(mapping->view, mapping->size) != 0)
    {
        return errno;
    }

    return 0;
}
