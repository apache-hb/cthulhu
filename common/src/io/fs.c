#include "fs/common.h"

#include "base/memory.h"
#include "base/panic.h"

#include "std/map.h"
#include "std/str.h"
#include "std/vector.h"

#include "report/report.h"

#include <string.h>

static inode_t *query_inode(fs_t *fs, inode_t *node, const char *name)
{
    CTASSERT(fs->cb->fnQueryNode != NULL);
    if (str_equal(name, "."))
    {
        return node;
    }

    return fs->cb->fnQueryNode(fs, node, name);
}

static inode_t *create_dir_inode(fs_t *fs, inode_t *parent, const char *name)
{
    CTASSERT(fs->cb->fnCreateDir != NULL);
    return fs->cb->fnCreateDir(fs, parent, name);
}

static inode_t *create_file_inode(fs_t *fs, inode_t *parent, const char *name)
{
    CTASSERT(fs->cb->fnCreateFile != NULL);
    return fs->cb->fnCreateFile(fs, parent, name);
}

static io_t *get_file_handle(fs_t *fs, inode_t *parent, const char *name, file_flags_t flags)
{
    CTASSERT(fs->cb->fnGetHandle != NULL);
    return fs->cb->fnGetHandle(fs, parent, name, flags);
}

static map_t *get_dir_listing(fs_t *fs, inode_t *parent)
{
    CTASSERT(fs->cb->fnGetDir != NULL);
    return fs->cb->fnGetDir(fs, parent);
}

static void delete_file_inode(fs_t *fs, inode_t *parent, const char *name)
{
    CTASSERT(fs->cb->fnDeleteFile != NULL);
    fs->cb->fnDeleteFile(fs, parent, name);
}

static void delete_dir_inode(fs_t *fs, inode_t *parent, const char *name)
{
    CTASSERT(fs->cb->fnDeleteDir != NULL);
    fs->cb->fnDeleteDir(fs, parent, name);
}

static vector_t *split_path(const char *path)
{
    return str_split(path, NATIVE_PATH_SEPARATOR);
}

static inode_t *checked_mkdir(fs_t *fs, inode_t *parent, const char *name)
{
    inode_t *node = query_inode(fs, parent, name);
    if (inode_is(node, eNodeDir))
    {
        return node;
    }

    if (node != NULL)
    {
        logverbose("node `%s` is not a directory", name);
        return NULL;
    }

    return create_dir_inode(fs, parent, name);
}

static inode_t *checked_open(fs_t *fs, inode_t *parent, const char *name)
{
    inode_t *node = query_inode(fs, parent, name);
    if (inode_is(node, eNodeFile))
    {
        return node;
    }

    if (node != NULL)
    {
        return NULL;
    }

    return create_file_inode(fs, parent, name);
}

static inode_t *recursive_mkdir(fs_t *fs, vector_t *parts)
{
    size_t len = vector_len(parts);
    inode_t *node = fs->root;

    for (size_t i = 0; i < len; i++)
    {
        const char *part = vector_get(parts, i);
        inode_t *next = checked_mkdir(fs, node, part);

        if (next == NULL)
        {
            CTASSERTF(next != NULL, "failed to create dir %zu `%s`", i, part);
            return NULL; // TODO: error
        }

        node = next;
    }

    return node;
}

static inode_t *find_inode(fs_t *fs, inode_t *root, vector_t *parts)
{
    size_t len = vector_len(parts);
    inode_t *node = root;

    for (size_t i = 0; i < len; i++)
    {
        const char *part = vector_get(parts, i);
        inode_t *next = query_inode(fs, node, part);

        if (next == NULL)
        {
            return NULL;
        }

        node = next;
    }

    return node;
}

void fs_mkdir(fs_t *fs, const char *path)
{
    vector_t *parts = split_path(path);
    recursive_mkdir(fs, parts);
}

io_t *fs_open(fs_t *fs, const char *path, file_flags_t flags)
{
    char *dir = str_path(path);

    inode_t *inode = recursive_mkdir(fs, split_path(dir));
    CTASSERT(inode != NULL);

    char *name = str_filename(path);

    inode_t *file = checked_open(fs, inode, name);

    return get_file_handle(fs, file, name, flags);
}

static inode_t *get_inode_for_delete(fs_t *fs, vector_t *parts)
{
    inode_t *node = find_inode(fs, fs->root, parts);
    if (node == NULL) { return NULL; }
    if (node == fs->root) { return NULL; } // cant delete root dir, TODO: error

    return node;
}

void fs_rmdir(fs_t *fs, const char *path)
{
    vector_t *parts = split_path(path);
    inode_t *node = get_inode_for_delete(fs, parts);
    if (node == NULL) { return; }

    inode_t *parent = node->parent;
    const char *name = vector_tail(parts);

    if (node->type != eNodeDir) { return; } // TODO: error

    delete_dir_inode(fs, parent, name);
}

void fs_delete(fs_t *fs, const char *path)
{
    vector_t *parts = split_path(path);
    inode_t *node = get_inode_for_delete(fs, parts);
    if (node == NULL) { return; }

    inode_t *parent = node->parent;
    const char *name = vector_tail(parts);

    if (node->type != eNodeFile) { return; } // TODO: error

    delete_file_inode(fs, parent, name);
}

static void copy_file(io_t *from, io_t *to)
{
    io_seek(from, 0);
    io_seek(to, 0);
    
    uint8_t buffer[0x1000];
    size_t len = 0;

    while ((len = io_read(from, buffer, sizeof(buffer))) > 0)
    {
        logverbose("copy (len = %zu)", len);
        io_write(to, buffer, len);
    }

    io_close(from);
    io_close(to);
}

static void copy_recursive(fs_t *fs, inode_t *node, const char *path, fs_t *dst)
{
    logverbose("copy (%s)", path);
    map_t *dir = get_dir_listing(fs, node);
    map_iter_t iter = map_iter(dir);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        const char *name = entry.key;
        inode_t *child = entry.value;

        char *current = format("%s" NATIVE_PATH_SEPARATOR "%s", path, name);
        logverbose("current = %s", current);

        if (inode_is(child, eNodeFile))
        {
            logverbose("copy (from = %s)", current);
            io_t *to = fs_open(dst, current, eFileWrite | eFileBinary);
            io_t *from = get_file_handle(fs, child, current, eFileRead | eFileBinary);
            copy_file(from, to);
        }
        else if (inode_is(child, eNodeDir))
        {
            fs_mkdir(dst, current);
            copy_recursive(fs, child, current, dst);
        }
    }
}

void fs_copy(fs_t *fs, fs_t *dst)
{
    copy_recursive(fs, fs->root, ".", dst);
}
