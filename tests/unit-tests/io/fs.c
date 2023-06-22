#include "io/fs.h"
#include "report/report.h"

#include "std/str.h"

#include "ct-test.h"

TEST(test_fs_physical_creation, {
    reports_t *reports = begin_reports();
    OS_RESULT(const char *) cwd = os_dir_current();
    SHOULD_PASS("os_dir_current() should return a valid path", os_error(cwd) == 0);

    fs_t *fs = fs_physical(reports, format("%s" NATIVE_PATH_SEPARATOR "data" NATIVE_PATH_SEPARATOR "unit-test-data", OS_VALUE(const char *, cwd)));

    SHOULD_PASS("fs_physical() should return a valid fs_t pointer", fs != NULL);
})

TEST(test_fs_physical_sync, {
    reports_t *reports = begin_reports();

    OS_RESULT(const char *) cwd = os_dir_current();
    SHOULD_PASS("os_dir_current() should return a valid path", os_error(cwd) == 0);

    fs_t *fs = fs_physical(reports, format("%s" NATIVE_PATH_SEPARATOR "data" NATIVE_PATH_SEPARATOR "unit-test-data", OS_VALUE(const char *, cwd)));

    SHOULD_PASS("fs_physical() should return a valid fs_t pointer", fs != NULL);

    fs_t *vfs = fs_virtual(reports, "test");
    SHOULD_PASS("fs_virtual() should return a valid fs_t pointer", vfs != NULL);
    
    SHOULD_PASS("physical fs should have input1.txt", fs_file_exists(fs, "vfs/input1.txt"));
    SHOULD_PASS("physical fs should have temp.txt", fs_file_exists(fs, "vfs/next/nested/temp.txt"));
    SHOULD_PASS("physical fs should have empty subdir", fs_dir_exists(fs, "vfs/subdir"));

    fs_sync(vfs, fs);

    SHOULD_PASS("fs_sync() should sync directories", fs_dir_exists(vfs, "vfs/subdir"));
    SHOULD_PASS("fs_sync() should sync files", fs_file_exists(vfs, "vfs/input1.txt"));
    SHOULD_PASS("fs_sync() should sync files", fs_file_exists(vfs, "vfs/next/nested/temp.txt"));
})

HARNESS("pfs", {
    ENTRY("physical_new", test_fs_physical_creation),
    ENTRY("physical_sync", test_fs_physical_sync)
})
