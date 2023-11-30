#include "unit/ct-test.hpp"

#include "io/fs.h"
#include "report/report.h"
#include "std/str.h"

static const report_config_t kConfig = {
    SIZE_MAX,
    true
};

int main()
{
    test_suite_t::install_panic_handler();

    test_suite_t suite("fs");

    {
        reports_t *reports = begin_reports();

        suite.test_group("physical")
            .EXPECT_PANIC("no null name", fs_physical(reports, NULL))
            .EXPECT_PANIC("no null reports", fs_physical(NULL, "test"))
            .EXPECT_PANIC("null everything", fs_physical(NULL, NULL))
            .EXPECT_PASS("no return null", fs_physical(reports, "test") != NULL);
    }

    {
        test_group_t group = suite.test_group("current dir");
        reports_t *reports = begin_reports();
        OS_RESULT(const char *) cwd = os_dir_current();
        group.EXPECT_PASS("os_dir_current() should return a valid path", os_error(cwd) == 0);

        fs_t *fs = fs_physical(reports, format("%s" NATIVE_PATH_SEPARATOR "data" NATIVE_PATH_SEPARATOR "unit-test-data", OS_VALUE(const char *, cwd)));

        int err = end_reports(reports, "", kConfig);
        group.EXPECT_PASS("end_reports() should return 0", err == 0);

        group.EXPECT_PASS("fs_physical() should return a valid fs_t pointer", fs != NULL);
    }

    {
        test_group_t group = suite.test_group("other");
        reports_t *reports = begin_reports();

        OS_RESULT(const char *) cwd = os_dir_current();
        group.EXPECT_PASS("os_dir_current() should return a valid path", os_error(cwd) == 0);

        fs_t *fs = fs_physical(reports, format("%s" NATIVE_PATH_SEPARATOR "data" NATIVE_PATH_SEPARATOR "unit-test-data", OS_VALUE(const char *, cwd)));

        int err = end_reports(reports, "", kConfig);
        group.EXPECT_PASS("end_reports() should return 0", err == 0);

        group.EXPECT_PASS("fs_physical() should return a valid fs_t pointer", fs != NULL);

        fs_t *vfs = fs_virtual(reports, "test");
        group.EXPECT_PASS("fs_virtual() should return a valid fs_t pointer", vfs != NULL);

        group.EXPECT_PASS("physical fs should have input1.txt", fs_file_exists(fs, "vfs/input1.txt"));
        group.EXPECT_PASS("physical fs should have temp.txt", fs_file_exists(fs, "vfs/next/nested/temp.txt"));
        group.EXPECT_PASS("physical fs should have empty subdir", fs_dir_exists(fs, "vfs/subdir"));

        fs_sync(vfs, fs);

        group.EXPECT_PASS("fs_sync() should sync directories", fs_dir_exists(vfs, "vfs/subdir"));
        group.EXPECT_PASS("fs_sync() should sync files", fs_file_exists(vfs, "vfs/input1.txt"));
        group.EXPECT_PASS("fs_sync() should sync files", fs_file_exists(vfs, "vfs/next/nested/temp.txt"));

        err = end_reports(reports, "", kConfig);
        group.EXPECT_PASS("end_reports() should return 0", err == 0);
    }

    return suite.finish_suite();
}
