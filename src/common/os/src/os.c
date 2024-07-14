// SPDX-License-Identifier: LGPL-3.0-only

#include "os/os.h"
#include "os_common.h"

#include "base/util.h"

#include "arena/arena.h"
#include "base/panic.h"

///
/// init/exit functions
///

void os_init(void)
{
    impl_init();
}

CT_NORETURN os_exit(os_exitcode_t code)
{
    impl_exit(code);
    CT_UNREACHABLE();
}

CT_NORETURN os_thread_exit(os_status_t status)
{
    impl_thread_exit(status);
    CT_UNREACHABLE();
}

CT_NORETURN os_abort(void)
{
    impl_abort();
    CT_UNREACHABLE();
}

///
/// to string operations
///

STA_DECL
size_t os_error_get_string(os_error_t error, char *buffer, size_t size)
{
    if (size == 0)
    {
        CTASSERT(buffer == NULL);
        return impl_error_length(error);
    }

    CTASSERT(buffer != NULL);
    return impl_error_string(error, buffer, size);
}

STA_DECL
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

STA_DECL
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

STA_DECL
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

///
/// mapping operations
///

STA_DECL
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

    return err;
}

STA_DECL
os_error_t os_unmap(os_mapping_t *mapping)
{
    CTASSERT(mapping != NULL);

    return impl_unmap(mapping);
}

STA_DECL
size_t os_mapping_size(const os_mapping_t *mapping)
{
    CTASSERT(mapping != NULL);
    CTASSERT(os_mapping_active(mapping));

    return mapping->size;
}

STA_DECL
void *os_mapping_data(os_mapping_t *mapping)
{
    CTASSERT(mapping != NULL);
    CTASSERT(os_mapping_active(mapping));

    return mapping->view;
}

STA_DECL
bool os_mapping_active(const os_mapping_t *mapping)
{
    CTASSERT(mapping != NULL);

    return mapping->view != CT_OS_INVALID_MAPPING;
}

///
/// shared libraries
///

STA_DECL
os_error_t os_library_open(const char *path, os_library_t *library)
{
    CTASSERT(path != NULL);
    CTASSERTF(library != NULL, "invalid library handle used for %s", path);

    os_library_impl_t lib = impl_library_open(path);
    if (lib == CT_OS_INVALID_LIBRARY)
    {
        return impl_last_error();
    }

    library->impl = lib;
    library->name = path;

    return eOsSuccess;
}

STA_DECL
os_error_t os_library_close(os_library_t *library)
{
    CTASSERT(library != NULL);

    if (!impl_library_close(library->impl))
    {
        return impl_last_error();
    }

    return eOsSuccess;
}

STA_DECL
os_error_t os_library_symbol(os_library_t *library, void **symbol, const char *name)
{
    CTASSERT(library != NULL);
    CTASSERT(name != NULL);

    void *addr = impl_library_symbol(library->impl, name);
    if (addr == NULL)
    {
        return impl_last_error();
    }

    *symbol = addr;

    return eOsSuccess;
}

STA_DECL
const char *os_library_name(const os_library_t *library)
{
    CTASSERT(library != NULL);

    return library->name;
}

///
/// file operations
///

STA_DECL
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

    file->path = path;
    file->impl = fd;

    return eOsSuccess;
}

STA_DECL
os_error_t os_file_close(os_file_t *file)
{
    CTASSERT(file != NULL);
    CTASSERTF(file->impl != CT_OS_INVALID_FILE, "invalid file handle (%s)", file->path);

    if (!impl_file_close(file->impl))
    {
        return impl_last_error();
    }

    return eOsSuccess;
}

STA_DECL
const char *os_file_name(const os_file_t *file)
{
    CTASSERT(file != NULL);

    return file->path;
}

STA_DECL
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

STA_DECL
os_error_t os_file_copy(const char *dst, const char *src)
{
#if CTU_OS_HAS_COPYFILE
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

STA_DECL
bool os_dir_exists(const char *path)
{
    os_dirent_t type = os_dirent_type(path);
    return type == eOsNodeDir;
}

///
/// directory iteration operations
///

static void inode_init(os_inode_t *dst, const os_inode_impl_t *src)
{
    dst->type = impl_inode_type(src);
    ctu_strcpy(dst->name, impl_inode_name(src), CT_OS_NAME_MAX);
}

static bool iter_next(os_iter_t *iter, os_inode_impl_t *result)
{
    bool ok = impl_iter_next(iter->impl, result);
    if (!ok)
    {
        iter->error = impl_last_error();
    }

    return ok;
}

STA_DECL
os_error_t os_iter_begin(const char *path, os_iter_t *iter)
{
    CTASSERT(path != NULL);
    CTASSERT(iter != NULL);

    iter->error = eOsSuccess;
    iter->impl = impl_iter_open(path, &iter->current);
    if (iter->impl == CT_OS_INVALID_ITER)
    {
        iter->error = impl_last_error();
    }

    return iter->error;
}

STA_DECL
os_error_t os_iter_end(os_iter_t *iter)
{
    CTASSERT(iter != NULL);

    if (!impl_iter_close(iter->impl))
    {
        return impl_last_error();
    }

    return eOsSuccess;
}

STA_DECL
bool os_iter_next(os_iter_t *iter, os_inode_t *dir)
{
    CTASSERT(iter != NULL);
    CTASSERT(dir != NULL);

    if (iter->error != eOsSuccess)
        return false;

    os_inode_impl_t data;
    while (iter_next(iter, &data))
    {
        if (is_path_special(impl_inode_name(&data)))
        {
            continue;
        }

        break;
    }

    if (iter->error != eOsSuccess)
        return false;

    inode_init(dir, &data);
    return true;
}

STA_DECL
os_error_t os_iter_error(const os_iter_t *iter)
{
    CTASSERT(iter != NULL);

    return iter->error;
}

///
/// inode operations
///

STA_DECL
os_dirent_t os_inode_type(const os_inode_t *node)
{
    CTASSERT(node != NULL);

    return node->type;
}

STA_DECL
const char *os_inode_name(const os_inode_t *node)
{
    CTASSERT(node != NULL);

    return node->name;
}

///
/// thread operations
///

STA_DECL
const char *os_thread_name(const os_thread_t *thread)
{
    CTASSERT(thread != NULL);

    return thread->name;
}

STA_DECL
bool os_thread_cmpid(const os_thread_t *thread, os_thread_id_t id)
{
    CTASSERT(thread != NULL);

    return thread->id == id;
}
