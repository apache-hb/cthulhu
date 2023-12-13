#include "unit/ct-test.h"

#include "fs/fs.h"
#include "report/report.h"

int main()
{
    test_install_panic_handler();

    test_suite_t suite = test_suite_new("vfs");

    // virtual
    {
        reports_t *reports = begin_reports();

        test_group_t group = test_group(&suite, "virtual");
        GROUP_EXPECT_PANIC(group, "no null name", fs_virtual(reports, NULL));
        GROUP_EXPECT_PANIC(group, "no null reports", fs_virtual(NULL, "test"));
        GROUP_EXPECT_PANIC(group, "null everything", fs_virtual(NULL, NULL));
        GROUP_EXPECT_PASS(group, "no return null", fs_virtual(reports, "test") != NULL);
    }

    // mkdir
    {
        test_group_t group = test_group(&suite, "mkdir");
        reports_t *reports = begin_reports();
        fs_t *fs = fs_virtual(reports, "test");
        fs_dir_create(fs, "testdir/foo/bar");

        GROUP_EXPECT_PASS(group, "fs_dir_create() should recursively create dirs", fs_dir_exists(fs, "testdir"));
        GROUP_EXPECT_PASS(group, "fs_dir_create() should recursively create dirs", fs_dir_exists(fs, "testdir/foo"));
        GROUP_EXPECT_PASS(group, "fs_dir_exists() should return true for a directory that exists", fs_dir_exists(fs, "testdir/foo/bar"));

        GROUP_EXPECT_PASS(group, "fs_dir_exists() should return false for a directory that does not exist", !fs_dir_exists(fs, "testdir/foo/bar/baz"));
    }

    // rmdir
    {
        test_group_t group = test_group(&suite, "rmdir");
        reports_t *reports = begin_reports();
        fs_t *fs = fs_virtual(reports, "test");
        fs_dir_create(fs, "testdir/foo/bar");
        fs_dir_delete(fs, "testdir/foo/bar");

        GROUP_EXPECT_FAIL(group, "fs_dir_delete() should remove dir", fs_dir_exists(fs, "testdir/foo/bar"));
        GROUP_EXPECT_PASS(group, "fs_dir_delete() should remove dir", fs_dir_exists(fs, "testdir/foo"));
        GROUP_EXPECT_PASS(group, "fs_dir_delete() should remove dir", fs_dir_exists(fs, "testdir"));
    }

    // sync
    {
        test_group_t group = test_group(&suite, "sync");
        reports_t *reports = begin_reports();
        fs_t *dst = fs_virtual(reports, "dst");
        fs_t *src = fs_virtual(reports, "src");

        fs_dir_create(src, "testdir/foo/bar");

        fs_sync(dst, src);

        GROUP_EXPECT_PASS(group, "fs_sync() should sync directories", fs_dir_exists(dst, "testdir/foo/bar"));
    }

    // cleanup the physical test directory
    os_dir_delete("test");

    return test_suite_finish(&suite);
}
