#include "../../../src/driver/cmd.h"
#include "cthulhu/util/report.h"
#include "test.h"

TEST(test_cmd_files, {
    reports_t *reports = begin_reports();
    commands_t commands;
    const char *argv[] = { "test-harness", "hello.test" };
    int argc = 2;
    int result = parse_commandline(reports, &commands, argc, argv);
    
    SHOULD_PASS("parse one file status", result == 0);
    SHOULD_PASS("has one file", vector_len(commands.sources) == 1);
    SHOULD_PASS("has correct file", str_equal(vector_get(commands.sources, 0), "hello.test") );
})

HARNESS("cmd", {
    ENTRY("file input", test_cmd_files)
})
