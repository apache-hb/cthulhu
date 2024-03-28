// SPDX-License-Identifier: LGPL-3.0-only

#include "os/os.h"
#include "os_common.h"

#include "base/panic.h"
#include "core/macros.h"

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
os_error_t os_file_exists(const char *path)
{
    CTASSERT(path != NULL);

    if (access(path, F_OK) == 0)
    {
        return eOsExists;
    }

    return errno;
}

os_file_impl_t impl_file_open(const char *path, os_access_t access)
{
    return fopen(path, get_access(access));
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

bool impl_file_close(os_file_t *file)
{
    return fclose(file->file) == 0;
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

void *impl_file_map(os_file_t *file, os_protect_t protect, size_t size, os_mapping_t *mapping)
{
    CT_UNUSED(mapping);

    int prot = get_mmap_prot(protect);

    int fd = fileno(file->file);
    return mmap(NULL, size, prot, MAP_PRIVATE, fd, 0);
}

os_error_t impl_unmap(os_mapping_t *map)
{
    if (munmap(map->view, map->size) != 0)
    {
        return errno;
    }

    return 0;
}
