#include "memory/memory.h"
#include "unit/ct-test.h"

#include "fs/fs.h"

#include "std/str.h"

int main(void)
{
    test_install_panic_handler();

    arena_t *arena = ctu_default_alloc();
    test_suite_t suite = test_suite_new("fs");

    {
        test_group_t group = test_group(&suite, "physical");
        GROUP_EXPECT_PANIC(group, "no null name", fs_physical(NULL, arena));
        GROUP_EXPECT_PASS(group, "no return null", fs_physical("test", arena) != NULL);
    }

    {
        test_group_t group = test_group(&suite, "current dir");
        OS_RESULT(const char *) cwd = os_dir_current();
        GROUP_EXPECT_PASS(group, "os_dir_current() should return a valid path", os_error(cwd) == 0);

        fs_t *fs = fs_physical(format("%s" NATIVE_PATH_SEPARATOR "data" NATIVE_PATH_SEPARATOR "unit-test-data", OS_VALUE(const char *, cwd)), arena);

        GROUP_EXPECT_PASS(group, "fs_physical() should return a valid fs_t pointer", fs != NULL);
    }

    {
        test_group_t group = test_group(&suite, "other");

        OS_RESULT(const char *) cwd = os_dir_current();
        GROUP_EXPECT_PASS(group, "os_dir_current() should return a valid path", os_error(cwd) == 0);

        fs_t *fs = fs_physical(format("%s" NATIVE_PATH_SEPARATOR "data" NATIVE_PATH_SEPARATOR "unit-test-data", OS_VALUE(const char *, cwd)), arena);

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

    // cleanup the physical test directory
    os_dir_delete("test");

    return test_suite_finish(&suite);
}
