#include "io/io.h"
#include "unit/ct-test.h"

#include "arena/arena.h"

#include "setup/memory.h"

#include "os/os.h"
#include "fs/fs.h"

#include "std/str.h"

int main(void)
{
    test_install_panic_handler();
    test_install_electric_fence();

    arena_t *arena = ctu_default_alloc();
    test_suite_t suite = test_suite_new("fs", arena);

    text_t cwd = {0};
    os_error_t err = os_getcwd(&cwd, arena);
    CTASSERTF(err == eOsSuccess, "os_getcwd() failed with error %s", os_error_string(err, arena));

    {
        test_group_t group = test_group(&suite, "physical");
        GROUP_EXPECT_PANIC(group, "no null name", (void)fs_physical(NULL, arena));
        GROUP_EXPECT_PASS(group, "no return null", fs_physical("./test", arena) != NULL);
    }

    {
        test_group_t group = test_group(&suite, "current dir");

        fs_t *fs = fs_physical(str_format(arena, "%s" CT_NATIVE_PATH_SEPARATOR "data" CT_NATIVE_PATH_SEPARATOR "unit-test-data", cwd.text), arena);

        GROUP_EXPECT_PASS(group, "fs_physical() should return a valid fs_t pointer", fs != NULL);
    }

    {
        test_group_t group = test_group(&suite, "other");

        fs_t *fs = fs_physical(str_format(arena, "%s" CT_NATIVE_PATH_SEPARATOR "data" CT_NATIVE_PATH_SEPARATOR "unit-test-data", cwd.text), arena);

        GROUP_EXPECT_PASS(group, "fs_physical() should return a valid fs_t pointer", fs != NULL);

        fs_t *vfs = fs_virtual("test", arena);
        GROUP_EXPECT_PASS(group, "fs_virtual() should return a valid fs_t pointer", vfs != NULL);

        GROUP_EXPECT_PASS(group, "physical fs should have input1.txt", fs_file_exists(fs, "vfs/input1.txt"));
        GROUP_EXPECT_PASS(group, "physical fs should have temp.txt", fs_file_exists(fs, "vfs/next/nested/temp.txt"));
        GROUP_EXPECT_PASS(group, "physical fs should have empty subdir", fs_dir_exists(fs, "vfs/subdir"));

        sync_result_t result = fs_sync(vfs, fs);

        GROUP_EXPECT_PASS(group, "result should be empty", result.path == NULL);
        GROUP_EXPECT_PASS(group, "fs_sync() should sync directories", fs_dir_exists(vfs, "vfs/subdir"));
        GROUP_EXPECT_PASS(group, "fs_sync() should sync files", fs_file_exists(vfs, "vfs/input1.txt"));
        GROUP_EXPECT_PASS(group, "fs_sync() should sync files", fs_file_exists(vfs, "vfs/next/nested/temp.txt"));
    }

    {
        test_group_t group = test_group(&suite, "file manipulation");

        fs_t *fs = fs_physical(str_format(arena, "%s" CT_NATIVE_PATH_SEPARATOR "data" CT_NATIVE_PATH_SEPARATOR "unit-test-data", cwd.text), arena);

        GROUP_EXPECT_PASS(group, "fs_physical() should return a valid fs_t pointer", fs != NULL);

        fs_file_create(fs, "hello.txt");

        io_t *io = fs_open(fs, "hello.txt", eOsAccessWrite);

        GROUP_EXPECT_PASS(group, "fs_open() should return a valid io_t pointer", io != NULL);
        GROUP_EXPECT_PASS(group, "io_error() should return no error", io_error(io) == eOsSuccess);
    }

    // cleanup the physical test directory
    os_dir_delete("test");

    return test_suite_finish(&suite);
}
