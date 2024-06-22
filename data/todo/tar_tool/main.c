// SPDX-License-Identifier: GPL-3.0-only

#include "base/panic.h"
#include "base/util.h"
#include "core/macros.h"
#include "fs/fs.h"
#include "io/console.h"
#include "io/io.h"
#include "setup/memory.h"
#include "setup/setup.h"

#include "std/str.h"
#include "tar/tar.h"

#include "os/os.h"

// we do a few assumes that have side effects
// they dont *actually* have side effects, but they arent pure either
#if defined(__clang__)
CT_PRAGMA(clang diagnostic push)
CT_PRAGMA(clang diagnostic ignored "-Wassume")
#endif

static void print_dirent(fs_t *fs, const fs_inode_t *inode, const char *dir);

static void print_folder(fs_t *fs, const fs_inode_t *inode, const char *dir)
{
    io_t *con = io_stdout();
    arena_t *arena = ctu_default_alloc();
    const char *name = fs_inode_name(inode);
    os_dirent_t type = fs_inode_type(inode);

    char *path = str_format(arena, "%s/%s", dir, name);

    io_printf(con, "%s/%s (%s)\n", dir, name, os_dirent_string(type));

    if (type == eOsNodeDir)
    {
        CTASSERTF(fs_dir_exists(fs, path), "directory `%s` does not exist", path);

        fs_iter_t *iter;
        fs_inode_t *child;
        os_error_t err = fs_iter_begin(fs, inode, &iter);
        CTASSERTF(err == eOsSuccess, "failed to begin iteration");

        while (fs_iter_next(iter, &child) == eOsSuccess)
        {
            print_dirent(fs, inode, path);
        }

        fs_iter_end(iter);
    }
}

static void print_dirent(fs_t *fs, const fs_inode_t *inode, const char *dir)
{
    CT_UNUSED(dir);

    io_t *con = io_stdout();
    arena_t *arena = ctu_default_alloc();
    const char *name = fs_inode_name(inode);
    os_dirent_t type = fs_inode_type(inode);

    char *path = str_format(arena, "%s/%s", dir, name);

    io_printf(con, "%s/%s (%s)\n", dir, name, os_dirent_string(type));

    if (type == eOsNodeDir)
        CTASSERTF(fs_dir_exists(fs, path), "directory `%s` does not exist", path);

    if (type != eOsNodeFile)
    {
        print_folder(fs, inode, path);
        return;
    }

    CTASSERTF(fs_file_exists(fs, path), "file `%s` does not exist", path);

    io_t *io = fs_open(fs, path, eOsAccessRead);
    os_error_t err = io_error(io);
    CTASSERTF(err == 0, "failed to open file `%s` (%s)", path, os_error_string(err, arena));

    size_t size = io_size(io);

    io_printf(con, " - size: %zu\n", size);

    io_close(io);
}

int main(int argc, const char **argv)
{
    setup_default(NULL);
    arena_t *arena = ctu_default_alloc();

    CTASSERTF(argc > 2, "usage: %s input.tar", argv[0]);

    const char *input = argv[1];
    io_t *io = io_file(input, eOsAccessRead, arena);
    os_error_t err = io_error(io);
    CTASSERTF(err == 0, "failed to open file `%s` (%s)", input, os_error_string(err, arena));

    io_t *con = io_stdout();

    fs_t *dst = fs_virtual("data", arena);
    tar_result_t result = tar_extract(dst, io);
    switch (result.error)
    {
    case eTarWriteError:
        io_printf(con, "write error, expected to read %zu bytes, read %zu\n", result.expected, result.actual);
        return 1;
    case eTarReadError:
        io_printf(con, "read error, expected to read %zu bytes, read %zu\n", result.expected, result.actual);
        return 1;
    case eTarUnknownEntry:
        io_printf(con, "invalid entry type `%c` (%d)\n", result.type, result.type);
        return 1;
    case eTarInvalidHeader:
        io_printf(con, "invalid header\n");
        return 1;

    case eTarInvalidDirName:
        io_printf(con, "directory name `%s` would erase existing file\n", result.name);
        return 1;

    default:
        break;
    }

    io_free(io);

    if (str_equal(argv[2], "--list"))
    {
        fs_iter_t *iter;
        fs_inode_t *inode;
        os_error_t error = fs_iter_begin(dst, fs_root_inode(dst), &iter);
        CTASSERTF(error == eOsSuccess, "failed to begin iteration");

        while (fs_iter_next(iter, &inode) == eOsSuccess)
        {
            print_dirent(dst, inode, ".");
        }

        fs_iter_end(iter);
    }
}

#if defined(__clang__)
CT_PRAGMA(clang diagnostic pop)
#endif
