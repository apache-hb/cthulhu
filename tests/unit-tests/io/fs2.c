#include "io/fs.h"
#include "report/report.h"

#include "ct-test.h"

TEST(test_fs2_virtual_creation, {
    reports_t *reports = begin_reports();
    fs2_t *fs = fs2_virtual(reports, "test");
    SHOULD_PASS("fs_virtual() should return a valid fs2_t pointer", fs != NULL);
})

TEST(test_fs2_virtual_mkdir, {
    reports_t *reports = begin_reports();
    fs2_t *fs = fs2_virtual(reports, "test");
    fs2_dir_create(fs, "testdir/foo/bar");

    SHOULD_PASS("fs2_dir_create() should recursively create dirs", fs2_dir_exists(fs, "testdir"));
    SHOULD_PASS("fs2_dir_create() should recursively create dirs", fs2_dir_exists(fs, "testdir/foo"));
    SHOULD_PASS("fs2_dir_exists() should return true for a directory that exists", fs2_dir_exists(fs, "testdir/foo/bar"));

    SHOULD_PASS("fs2_dir_exists() should return false for a directory that does not exist", !fs2_dir_exists(fs, "testdir/foo/bar/baz"));
})

TEST(test_fs2_virtual_rmdir, {
    reports_t *reports = begin_reports();
    fs2_t *fs = fs2_virtual(reports, "test");
    fs2_dir_create(fs, "testdir/foo/bar");
    fs2_dir_delete(fs, "testdir/foo/bar");

    SHOULD_FAIL("fs2_dir_delete() should remove dir", fs2_dir_exists(fs, "testdir/foo/bar"));
    SHOULD_PASS("fs2_dir_delete() should remove dir", fs2_dir_exists(fs, "testdir/foo"));
    SHOULD_PASS("fs2_dir_delete() should remove dir", fs2_dir_exists(fs, "testdir"));
})

TEST(test_fs2_virtual_sync, {
    reports_t *reports = begin_reports();
    fs2_t *dst = fs2_virtual(reports, "dst");
    fs2_t *src = fs2_virtual(reports, "src");

    fs2_dir_create(src, "testdir/foo/bar");
    
    fs2_sync(dst, src);

    SHOULD_PASS("fs2_sync() should sync directories", fs2_dir_exists(dst, "testdir/foo/bar"));
})

HARNESS("fs2", {
    ENTRY("virtual_new", test_fs2_virtual_creation),
    ENTRY("virtual_mkdir", test_fs2_virtual_mkdir),
    ENTRY("virtual_rmdir", test_fs2_virtual_rmdir),
    ENTRY("virtual_sync", test_fs2_virtual_sync)
})
