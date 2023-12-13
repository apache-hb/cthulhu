#include "unit/ct-test.h"

#include "fs/fs.h"

#include "report/report.h"
#include "std/str.h"

#include <stdint.h>

static const report_config_t kConfig = {
    .limit = SIZE_MAX,
    .warningsAreErrors = true
};

int main()
{
    test_install_panic_handler();

    test_suite_t suite = test_suite_new("fs");

    {
        reports_t *reports = begin_reports();

        test_group_t group = test_group(&suite, "physical");
        GROUP_EXPECT_PANIC(group, "no null name", fs_physical(reports, NULL));
        GROUP_EXPECT_PANIC(group, "no null reports", fs_physical(NULL, "test"));
        GROUP_EXPECT_PANIC(group, "null everything", fs_physical(NULL, NULL));
        GROUP_EXPECT_PASS(group, "no return null", fs_physical(reports, "test") != NULL);
    }

    {
        test_group_t group = test_group(&suite, "current dir");
        reports_t *reports = begin_reports();
        OS_RESULT(const char *) cwd = os_dir_current();
        GROUP_EXPECT_PASS(group, "os_dir_current() should return a valid path", os_error(cwd) == 0);

        fs_t *fs = fs_physical(reports, format("%s" NATIVE_PATH_SEPARATOR "data" NATIVE_PATH_SEPARATOR "unit-test-data", OS_VALUE(const char *, cwd)));

        int err = end_reports(reports, "", kConfig);
        GROUP_EXPECT_PASS(group, "end_reports() should return 0", err == 0);

        GROUP_EXPECT_PASS(group, "fs_physical() should return a valid fs_t pointer", fs != NULL);
    }

    {
        test_group_t group = test_group(&suite, "other");
        reports_t *reports = begin_reports();

        OS_RESULT(const char *) cwd = os_dir_current();
        GROUP_EXPECT_PASS(group, "os_dir_current() should return a valid path", os_error(cwd) == 0);

        fs_t *fs = fs_physical(reports, format("%s" NATIVE_PATH_SEPARATOR "data" NATIVE_PATH_SEPARATOR "unit-test-data", OS_VALUE(const char *, cwd)));

        int err = end_reports(reports, "", kConfig);
        GROUP_EXPECT_PASS(group, "end_reports() should return 0", err == 0);

        GROUP_EXPECT_PASS(group, "fs_physical() should return a valid fs_t pointer", fs != NULL);

        fs_t *vfs = fs_virtual(reports, "test");
        GROUP_EXPECT_PASS(group, "fs_virtual() should return a valid fs_t pointer", vfs != NULL);

        GROUP_EXPECT_PASS(group, "physical fs should have input1.txt", fs_file_exists(fs, "vfs/input1.txt"));
        GROUP_EXPECT_PASS(group, "physical fs should have temp.txt", fs_file_exists(fs, "vfs/next/nested/temp.txt"));
        GROUP_EXPECT_PASS(group, "physical fs should have empty subdir", fs_dir_exists(fs, "vfs/subdir"));

        fs_sync(vfs, fs);

        GROUP_EXPECT_PASS(group, "fs_sync() should sync directories", fs_dir_exists(vfs, "vfs/subdir"));
        GROUP_EXPECT_PASS(group, "fs_sync() should sync files", fs_file_exists(vfs, "vfs/input1.txt"));
        GROUP_EXPECT_PASS(group, "fs_sync() should sync files", fs_file_exists(vfs, "vfs/next/nested/temp.txt"));

        err = end_reports(reports, "", kConfig);
        GROUP_EXPECT_PASS(group, "end_reports() should return 0", err == 0);
    }

    // cleanup the physical test directory
    os_dir_delete("test");

    return test_suite_finish(&suite);
}
