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
/// file operations
///

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
