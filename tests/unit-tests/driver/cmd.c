#include "../../../src/driver/cmd.h"
#include "cthulhu/util/report.h"
#include "test.h"

TEST(test_cmd_files, {
    reports_t *reports = begin_reports();
    commands_t commands = { 0 };
    int result;
    {
        const char *argv[] = { "test-harness", "hello.test" };
        int argc = 2;
        result = parse_commandline(reports, &commands, argc, argv);
    }

    SHOULD_PASS("parse one file status", result == 0);
    SHOULD_PASS("has one file", vector_len(commands.sources) == 1);
    SHOULD_PASS("has correct file", str_equal(vector_get(commands.sources, 0), "hello.test"));

    {
        const char *argv[] = { "test-harness", "hello.test", "world.test", "foo.bar" };
        int argc = 4;
        result = parse_commandline(reports, &commands, argc, argv);
    }

    SHOULD_PASS("parse two files status", result == 0);
    SHOULD_PASS("has 3 files", vector_len(commands.sources) == 3);
    SHOULD_PASS("has correct first file", str_equal(vector_get(commands.sources, 0), "hello.test"));
    SHOULD_PASS("has correct second file", str_equal(vector_get(commands.sources, 1), "world.test"));
    SHOULD_PASS("has correct third file", str_equal(vector_get(commands.sources, 2), "foo.bar"));
})

HARNESS("cmd", {
    ENTRY("file input", test_cmd_files)
})
