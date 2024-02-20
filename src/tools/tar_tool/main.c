#include "base/panic.h"
#include "core/macros.h"
#include "fs/fs.h"
#include "io/console.h"
#include "io/io.h"
#include "setup/memory.h"
#include "setup/setup.h"

#include "std/str.h"
#include "tar/tar.h"

static void print_dirent(const char *dir, const char *name, os_dirent_t type, void *user)
{
    CT_UNUSED(dir);

    fs_t *fs = user;
    io_t *con = io_stdout();
    arena_t *arena = ctu_default_alloc();

    char *path = str_format(arena, "%s/%s", dir, name);

    io_printf(con, "%s/%s (%s)\n", dir, name, os_dirent_string(type));

    if (type == eOsNodeDir)
        CTASSERTF(fs_dir_exists(fs, path), "directory `%s` does not exist", path);

    if (type != eOsNodeFile)
    {
        return;
    }

    CTASSERTF(fs_file_exists(fs, path), "file `%s` does not exist", path);

    io_t *io = fs_open(fs, path, eOsAccessRead);
    io_error_t err = io_error(io);
    CTASSERTF(err == 0, "failed to open file `%s` (%s)", path, os_error_string(err, arena));

    size_t size = io_size(io);

    io_printf(con, " - size: %zu\n", size);

    io_close(io);
}

int main(int argc, const char **argv)
{
    setup_global();
    arena_t *arena = ctu_default_alloc();

    CTASSERTF(argc >= 2, "usage: %s input.tar", argv[0]);

    const char *input = argv[1];
    io_t *io = io_file(input, eOsAccessRead, arena);
    io_error_t err = io_error(io);
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

    io_close(io);

    fs_iter_dirents(dst, ".", dst, print_dirent);
}
