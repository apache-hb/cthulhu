#include "tar/tar.h"
#include "ct-test.h"

#include "fs/fs.h"
#include "io/io.h"
#include "setup/memory.h"

int main(void)
{
    test_install_panic_handler();
    test_install_electric_fence();

    arena_t *arena = ctu_default_alloc();
    test_suite_t suite = test_suite_new("tar", arena);

    {
        test_group_t group = test_group(&suite, "round trip");

        fs_t *fs = fs_virtual("test", arena);
        fs_dir_create(fs, "dir1");
        fs_dir_create(fs, "dir2");

        fs_dir_create(fs, "dir1/inner");
        fs_file_create(fs, "dir1/inner/file.txt");

        io_t *io = fs_open(fs, "dir1/inner/file.txt", eOsAccessWrite | eOsAccessRead);
        io_write(io, "inner file", 5);
        io_close(io);

        fs_file_create(fs, "dir1/file.txt");
        fs_file_create(fs, "dir2/file.txt");
        fs_file_create(fs, "root.txt");

        io = fs_open(fs, "dir1/file.txt", eOsAccessWrite | eOsAccessRead);
        io_write(io, "hello", 5);
        io_close(io);

        io = fs_open(fs, "dir2/file.txt", eOsAccessWrite | eOsAccessRead);
        io_write(io, "world", 5);
        io_close(io);

        io = fs_open(fs, "root.txt", eOsAccessWrite | eOsAccessRead);
        io_write(io, "root", 4);
        io_close(io);

        io_t *dst = io_blob("out.tar", 0x1000, eOsAccessWrite | eOsAccessRead, arena);

        tar_error_t err = tar_archive(dst, fs, arena);
        GROUP_EXPECT_PASS(group, "archive", err == eTarOk);

        size_t size = io_size(dst);
        char *data = io_map(dst, eOsProtectRead);

        io_t *tmp = io_file("test.tar", eOsAccessWrite | eOsAccessTruncate, arena);
        io_write(tmp, data, size);
        io_close(tmp);

        fs_t *result = fs_virtual("result", arena);

        io_seek(dst, 0);

        tar_result_t extract = tar_extract(result, dst);
        GROUP_EXPECT_PASS(group, "extract error", extract.error == eTarOk);

        GROUP_EXPECT_PASS(group, "./dir1", fs_dir_exists(result, "dir1"));
        GROUP_EXPECT_PASS(group, "dir1/file.txt", fs_file_exists(result, "dir1/file.txt"));
        GROUP_EXPECT_PASS(group, "dir2", fs_dir_exists(result, "dir2"));
        GROUP_EXPECT_PASS(group, "dir2/file.txt", fs_file_exists(result, "dir2/file.txt"));
        GROUP_EXPECT_PASS(group, "root.txt", fs_file_exists(result, "root.txt"));
    }

    return test_suite_finish(&suite);
}
