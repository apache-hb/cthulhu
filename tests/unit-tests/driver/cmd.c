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

TEST(test_args, {
    reports_t *reports = begin_reports();
    commands_t commands = { 0 };
    int result;
    {
        const char *argv[] = { "test-harness", "--help" };
        int argc = 2;
        result = parse_commandline(reports, &commands, argc, argv);
    }

    SHOULD_PASS("parse one help status", result == 0);
    SHOULD_PASS("has no files", vector_len(commands.sources) == 0);
    SHOULD_PASS("has help", commands.printHelp);

    {
        const char *argv[] = { "test-harness", "-h", "--help" };
        int argc = 3;
        result = parse_commandline(reports, &commands, argc, argv);
    }

    SHOULD_PASS("parse two helps status", result == 0);
    SHOULD_PASS("has no files", vector_len(commands.sources) == 0);
    SHOULD_PASS("has help", commands.printHelp);

    {
        const char *argv[] = { "test-harness", "--version" };
        int argc = 2;
        result = parse_commandline(reports, &commands, argc, argv);
    }

    SHOULD_PASS("parse version status", result == 0);
    SHOULD_PASS("has no files", vector_len(commands.sources) == 0);
    SHOULD_PASS("has version", commands.printVersion);
})

TEST(test_multiple_args, {
    reports_t *reports = begin_reports();
    commands_t commands = { 0 };
    int result;

    {
        const char *argv[] = { "test-harness", "--help", "--version" };
        int argc = 3;
        result = parse_commandline(reports, &commands, argc, argv);
    }

    SHOULD_PASS("parse two args status", result == 0);
    SHOULD_PASS("has no files", vector_len(commands.sources) == 0);
    SHOULD_PASS("has help", commands.printHelp);
    SHOULD_PASS("has version", commands.printVersion);
})

TEST(test_string_arg, {
    reports_t *reports = begin_reports();
    commands_t commands = { 0 };
    int result;

    {
        const char *argv[] = { "test-harness", "-Bmodule", "foo" };
        int argc = 3;
        result = parse_commandline(reports, &commands, argc, argv);
    }

    SHOULD_PASS("parse module status", result == 0);
    SHOULD_PASS("module has no files", vector_len(commands.sources) == 0);
    SHOULD_PASS("has module name", str_equal(commands.moduleName, "foo"));

    {
        const char *argv[] = { "test-harness", "-Bmodule=foo" };
        int argc = 2;
        result = parse_commandline(reports, &commands, argc, argv);
    }

    SHOULD_PASS("parse module equals status", result == 0);
    SHOULD_PASS("module equals has no files", vector_len(commands.sources) == 0);
    SHOULD_PASS("has module name", str_equal(commands.moduleName, "foo"));
})

TEST(test_opt_and_string, {
    reports_t *reports = begin_reports();
    commands_t commands = { 0 };
    int result;

    {
        const char *argv[] = { "test-harness", "--help", "hello.test" };
        int argc = 3;
        result = parse_commandline(reports, &commands, argc, argv);
    }

    SHOULD_PASS("parse opt and string status", result == 0);
    SHOULD_PASS("has one files", vector_len(commands.sources) == 1);
    SHOULD_PASS("has help", commands.printHelp);
    SHOULD_PASS("has correct file", str_equal(vector_get(commands.sources, 0), "hello.test"));
})

HARNESS("cmd", {
    ENTRY("file input", test_cmd_files),
    ENTRY("args", test_args),
    ENTRY("multiple args", test_multiple_args),
    ENTRY("string arg", test_string_arg),
    ENTRY("opt and string", test_opt_and_string),
})
