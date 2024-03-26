// SPDX-License-Identifier: LGPL-3.0-only

#include "os/os.h"
#include "os_common.h"

#include "base/util.h"
#include "core/macros.h"

#include "arena/arena.h"
#include "base/panic.h"

///
/// to string operations
///

USE_DECL
char *os_error_string(os_error_t error, arena_t *arena)
{
    CTASSERT(arena != NULL);

    size_t size = os_error_get_string(error, NULL, 0);
    CTASSERT(size > 0);

    char *buffer = ARENA_MALLOC(size + 1, "os_error_string", NULL, arena);

    size_t written = os_error_get_string(error, buffer, size + 1);
    buffer[written] = '\0';

    return buffer;
}

USE_DECL
char *os_cwd_string(arena_t *arena)
{
    CTASSERT(arena != NULL);

    size_t size = os_cwd_get_string(NULL, 0);
    CTASSERT(size > 0);

    char *buffer = ARENA_MALLOC(size + 1, "os_getcwd", NULL, arena);

    size_t written = os_cwd_get_string(buffer, size + 1);
    buffer[written] = '\0';

    return buffer;
}

USE_DECL
os_error_t os_getcwd(text_t *text, arena_t *arena)
{
    CTASSERT(text != NULL);
    CTASSERT(arena != NULL);

    size_t size = os_cwd_get_string(NULL, 0);
    CTASSERT(size > 0);

    text->text = ARENA_MALLOC(size + 1, "os_getcwd", NULL, arena);
    text->length = size;

    size_t written = os_cwd_get_string(text->text, size + 1);
    if (written != size)
    {
        return impl_last_error();
    }

    return eOsSuccess;
}

USE_DECL
char *os_dir_string(const os_inode_t *dir, arena_t *arena)
{
    CTASSERT(dir != NULL);
    CTASSERT(arena != NULL);

    size_t size = os_dir_get_string(dir, NULL, 0);
    CTASSERT(size > 0);

    char *buffer = ARENA_MALLOC(size + 1, "os_dir_name", NULL, arena);

    size_t written = os_dir_get_string(dir, buffer, size + 1);
    buffer[written] = '\0';

    return buffer;
}

///
/// directory iteration operations
///

USE_DECL
size_t os_dir_get_string(const os_inode_t *dir, char *buffer, size_t size)
{
    CTASSERT(dir != NULL);

    if (size == 0)
    {
        CTASSERT(buffer == NULL);
        return impl_maxname();
    }

    CTASSERT(buffer != NULL);
    const char *name = impl_dirname(dir);
    size_t len = ctu_strlen(name);
    size_t copy = CT_MIN(len, size - 1);
    ctu_strcpy(buffer, name, copy);

    return len;
}

///
/// mapping operations
///

USE_DECL
os_error_t os_file_map(os_file_t *file, os_protect_t protect, size_t size, os_mapping_t *mapping)
{
    CTASSERT(file != NULL);
    CTASSERT(mapping != NULL);

    os_error_t err = eOsSuccess;

    if (size == 0)
    {
        err = os_file_size(file, &size);
        if (err != eOsSuccess)
        {
            return err;
        }
    }

    if (size == 0)
    {
        return eOsTooSmall;
    }

    mapping->size = size;
    void *ptr = impl_file_map(file, protect, size, mapping);
    if (ptr == CT_OS_INVALID_MAPPING)
    {
        err = impl_last_error();
    }
    else
    {
        mapping->view = ptr;
    }

    EVENT_MAPPING_OPEN(file, mapping);

    return err;
}

USE_DECL
os_error_t os_unmap(os_mapping_t *mapping)
{
    CTASSERT(mapping != NULL);

    EVENT_MAPPING_CLOSE(mapping);

    return impl_unmap(mapping);
}

USE_DECL
size_t os_mapping_size(const os_mapping_t *mapping)
{
    CTASSERT(mapping != NULL);
    CTASSERT(os_mapping_active(mapping));

    return mapping->size;
}

USE_DECL
void *os_mapping_data(os_mapping_t *mapping)
{
    CTASSERT(mapping != NULL);
    CTASSERT(os_mapping_active(mapping));

    return mapping->view;
}

USE_DECL
bool os_mapping_active(const os_mapping_t *mapping)
{
    CTASSERT(mapping != NULL);

    return mapping->view != CT_OS_INVALID_MAPPING;
}

///
/// shared libraries
///

USE_DECL
os_error_t os_library_open(const char *path, os_library_t *library)
{
    CTASSERT(path != NULL);
    CTASSERTF(library != NULL, "invalid library handle used for %s", path);

    os_library_impl_t lib = impl_library_open(path);
    if (lib == CT_OS_INVALID_LIBRARY)
    {
        return impl_last_error();
    }

    library->library = lib;
    library->name = path;

    EVENT_LIBRARY_OPEN(library);

    return eOsSuccess;
}

USE_DECL
os_error_t os_library_close(os_library_t *library)
{
    CTASSERT(library != NULL);

    EVENT_LIBRARY_CLOSE(library);

    if (!impl_library_close(library->library))
    {
        return impl_last_error();
    }

    return eOsSuccess;
}

USE_DECL
os_error_t os_library_symbol(os_library_t *library, os_symbol_t *symbol, const char *name)
{
    CTASSERT(library != NULL);
    CTASSERT(name != NULL);

    os_symbol_t addr = impl_library_symbol(library->library, name);
    if (addr == NULL)
    {
        return impl_last_error();
    }

    *symbol = addr;
    EVENT_LIBRARY_SYMBOL(library, name, symbol);

    return eOsSuccess;
}

USE_DECL
const char *os_library_name(const os_library_t *library)
{
    CTASSERT(library != NULL);

    return library->name;
}

///
/// file operations
///

USE_DECL
os_error_t os_file_open(const char *path, os_access_t access, os_file_t *file)
{
    CTASSERT(path != NULL);
    CTASSERT(file != NULL);
    CTASSERTF(access & (eOsAccessRead | eOsAccessWrite), "%s: invalid access flags 0x%x", path, access);
    CTASSERTF(access != (eOsAccessRead | eOsAccessTruncate), "%s: cannot truncate read only file", path);

    os_file_impl_t fd = impl_file_open(path, access);
    if (fd == CT_OS_INVALID_FILE)
    {
        return impl_last_error();
    }

    file->file = fd;
    file->path = path;

    EVENT_FILE_OPEN(file);

    return eOsSuccess;
}

USE_DECL
os_error_t os_file_close(os_file_t *fd)
{
    CTASSERT(fd != NULL);
    CTASSERTF(fd->file != CT_OS_INVALID_FILE, "invalid file handle (%s)", fd->path);

    EVENT_FILE_CLOSE(fd);

    if (!impl_file_close(fd))
    {
        return impl_last_error();
    }

    return eOsSuccess;
}

USE_DECL
const char *os_file_name(const os_file_t *file)
{
    CTASSERT(file != NULL);

    return file->path;
}

USE_DECL
os_error_t os_file_create(const char *path)
{
    CTASSERT(path != NULL);

    os_file_t fd = { 0 };
    os_error_t err = os_file_open(path, eOsAccessWrite | eOsAccessTruncate, &fd);
    if (err != eOsSuccess)
    {
        return err;
    }

    return os_file_close(&fd);
}
USE_DECL
os_error_t os_file_copy(const char *dst, const char *src)
{
#if CTU_OS_COPYFILE
    return impl_copyfile(dst, src);
#else
    os_file_t dstf = { 0 };
    os_file_t srcf = { 0 };
    os_error_t err = eOsSuccess;

    err = os_file_open(src, eOsAccessRead, &srcf);
    if (err != eOsSuccess)
    {
        goto cleanup;
    }

    err = os_file_open(dst, eOsAccessWrite | eOsAccessTruncate, &dstf);
    if (err != eOsSuccess)
    {
        goto cleanup;
    }

    char buffer[0x1000];
    size_t read = 0;
    size_t written = 0;

    while ((err = os_file_read(&srcf, buffer, sizeof(buffer), &read)) == eOsSuccess)
    {
        if (read == 0)
            break;

        err = os_file_write(&dstf, buffer, read, &written);
        if (err != eOsSuccess)
        {
            break;
        }
    }

cleanup:
    os_file_close(&srcf);
    os_file_close(&dstf);
    return err;
#endif
}

///
/// fs operations
///

USE_DECL
bool os_dir_exists(const char *path)
{
    os_dirent_t type = os_dirent_type(path);
    return type == eOsNodeDir;
}
