#include "unit/ct-test.hpp"

#include "fs/fs.h"
#include "report/report.h"

int main()
{
    test_suite_t::install_panic_handler();

    test_suite_t suite("vfs");

    // virtual
    {
        reports_t *reports = begin_reports();

        suite.test_group("virtual")
            .EXPECT_PANIC("no null name", fs_virtual(reports, NULL))
            .EXPECT_PANIC("no null reports", fs_virtual(NULL, "test"))
            .EXPECT_PANIC("null everything", fs_virtual(NULL, NULL))
            .EXPECT_PASS("no return null", fs_virtual(reports, "test") != NULL);
    }

    // mkdir
    {
        test_group_t group = suite.test_group("mkdir");
        reports_t *reports = begin_reports();
        fs_t *fs = fs_virtual(reports, "test");
        fs_dir_create(fs, "testdir/foo/bar");

        group.EXPECT_PASS("fs_dir_create() should recursively create dirs", fs_dir_exists(fs, "testdir"));
        group.EXPECT_PASS("fs_dir_create() should recursively create dirs", fs_dir_exists(fs, "testdir/foo"));
        group.EXPECT_PASS("fs_dir_exists() should return true for a directory that exists", fs_dir_exists(fs, "testdir/foo/bar"));

        group.EXPECT_PASS("fs_dir_exists() should return false for a directory that does not exist", !fs_dir_exists(fs, "testdir/foo/bar/baz"));
    }

    // rmdir
    {
        test_group_t group = suite.test_group("rmdir");
        reports_t *reports = begin_reports();
        fs_t *fs = fs_virtual(reports, "test");
        fs_dir_create(fs, "testdir/foo/bar");
        fs_dir_delete(fs, "testdir/foo/bar");

        group.EXPECT_FAIL("fs_dir_delete() should remove dir", fs_dir_exists(fs, "testdir/foo/bar"));
        group.EXPECT_PASS("fs_dir_delete() should remove dir", fs_dir_exists(fs, "testdir/foo"));
        group.EXPECT_PASS("fs_dir_delete() should remove dir", fs_dir_exists(fs, "testdir"));
    }

    // sync
    {
        test_group_t group = suite.test_group("sync");
        reports_t *reports = begin_reports();
        fs_t *dst = fs_virtual(reports, "dst");
        fs_t *src = fs_virtual(reports, "src");

        fs_dir_create(src, "testdir/foo/bar");

        fs_sync(dst, src);

        group.EXPECT_PASS("fs_sync() should sync directories", fs_dir_exists(dst, "testdir/foo/bar"));
    }

    return suite.finish_suite();
}
