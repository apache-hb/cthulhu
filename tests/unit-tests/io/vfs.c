#include "io/fs.h"
#include "report/report.h"

#include "ct-test.h"

TEST(test_fs_virtual_creation, {
    reports_t *reports = begin_reports();
    fs_t *fs = fs_virtual(reports, "test");
    SHOULD_PASS("fs_virtual() should return a valid fs_t pointer", fs != NULL);
})

TEST(test_fs_virtual_mkdir, {
    reports_t *reports = begin_reports();
    fs_t *fs = fs_virtual(reports, "test");
    fs_dir_create(fs, "testdir/foo/bar");

    SHOULD_PASS("fs_dir_create() should recursively create dirs", fs_dir_exists(fs, "testdir"));
    SHOULD_PASS("fs_dir_create() should recursively create dirs", fs_dir_exists(fs, "testdir/foo"));
    SHOULD_PASS("fs_dir_exists() should return true for a directory that exists", fs_dir_exists(fs, "testdir/foo/bar"));

    SHOULD_PASS("fs_dir_exists() should return false for a directory that does not exist", !fs_dir_exists(fs, "testdir/foo/bar/baz"));
})

TEST(test_fs_virtual_rmdir, {
    reports_t *reports = begin_reports();
    fs_t *fs = fs_virtual(reports, "test");
    fs_dir_create(fs, "testdir/foo/bar");
    fs_dir_delete(fs, "testdir/foo/bar");

    SHOULD_FAIL("fs_dir_delete() should remove dir", fs_dir_exists(fs, "testdir/foo/bar"));
    SHOULD_PASS("fs_dir_delete() should remove dir", fs_dir_exists(fs, "testdir/foo"));
    SHOULD_PASS("fs_dir_delete() should remove dir", fs_dir_exists(fs, "testdir"));
})

TEST(test_fs_virtual_sync, {
    reports_t *reports = begin_reports();
    fs_t *dst = fs_virtual(reports, "dst");
    fs_t *src = fs_virtual(reports, "src");

    fs_dir_create(src, "testdir/foo/bar");
    
    fs_sync(dst, src);

    SHOULD_PASS("fs_sync() should sync directories", fs_dir_exists(dst, "testdir/foo/bar"));
})

HARNESS("fs", {
    ENTRY("virtual_new", test_fs_virtual_creation),
    ENTRY("virtual_mkdir", test_fs_virtual_mkdir),
    ENTRY("virtual_rmdir", test_fs_virtual_rmdir),
    ENTRY("virtual_sync", test_fs_virtual_sync)
})
