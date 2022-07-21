#include "base/macros.h"
#include "base/memory.h"

#include "argparse/argparse.h" 
#include "report/report.h"
#include "std/vector.h"
#include "ct-test.h"

static const report_config_t kReportConfig = {
    .limit = SIZE_MAX,
    .warningsAreErrors = true
};

TEST(test_argparse_defaults, {
    reports_t *reports = begin_reports(&globalAlloc);
    
    const char *argv[] = { "argparse-test", "--help" };
    const int argc = 2;

    argparse_config_t config = {
        .argc = argc,
        .argv = argv,

        .description = "argparse-test",
        .version = NEW_VERSION(1, 0, 0),

        .reports = reports,
    
        .groups = vector_new(0)
    };

    argparse_t result = parse_args(&config);

    SHOULD_PASS("exitcode is 0", should_exit(&result));
    SHOULD_PASS("no errors", end_reports(reports, "", kReportConfig) == EXIT_OK);
    SHOULD_PASS("no extra files", vector_len(result.files) == 0);
})

TEST(test_unknown_arg, {
    reports_t *reports = begin_reports(&globalAlloc);
    
    const char *argv[] = { "argparse-test", "--helpaaaaa" };
    const int argc = 2;

    argparse_config_t config = {
        .argv = argv,
        .argc = argc,

        .description = "argparse-test",
        .version = NEW_VERSION(1, 0, 0),

        .reports = reports,
    
        .groups = vector_new(0)
    };

    argparse_t result = parse_args(&config);

    SHOULD_PASS("no extra files", vector_len(result.files) == 0);
})

TEST(test_extra_args, {
    reports_t *reports = begin_reports(&globalAlloc);

    const char *kParamNames[] = { "--param" };
    param_t *param = bool_param("test param", kParamNames, 1);
    group_t *group = new_group("test group", "test group desc", vector_init(param));
    
    const char *argv[] = { "argparse-test", "--help", "hello.txt", "--param" };
    const int argc = 4;

    argparse_config_t config = {
        .argv = argv,
        .argc = argc,

        .description = "argparse-test",
        .version = NEW_VERSION(1, 0, 0),

        .reports = reports,
    
        .groups = vector_init(group)
    };

    argparse_t result = parse_args(&config);

    SHOULD_PASS("no extra files", vector_len(result.files) == 1);
    SHOULD_PASS("has param", get_bool_arg(&result, param, false));
})

HARNESS("argparse", {
    ENTRY("default-args", test_argparse_defaults),
    ENTRY("unknown-arg", test_unknown_arg),
    ENTRY("extra-args", test_extra_args)
})
