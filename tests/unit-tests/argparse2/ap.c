#include "base/macros.h"
#include "base/memory.h"

#include "argparse2/argparse.h" 

#include "std/vector.h"
#include "std/str.h"

#include "ct-test.h"

static const report_config_t kReportConfig = {
    .limit = SIZE_MAX,
    .warningsAreErrors = true
};

typedef struct sources_t
{
    vector_t *files;
} sources_t;

typedef struct errors_t
{
    vector_t *errors;
} errors_t;

static ap_event_result_t on_file(ap_t *ap, const ap_param_t *param, const void *value, void *data)
{
    sources_t *sources = data;
    const char *path = value;

    if (str_equal(path, "file.txt"))
    {
        vector_push(&sources->files, (char*)path);
        return eEventHandled;
    }

    return eEventContinue;
}

static ap_event_result_t on_error(ap_t *ap, const node_t *node, const char *message, void *data)
{
    errors_t *errors = data;
    vector_push(&errors->errors, (char*)message);
    return eEventHandled;
}

TEST(test_argparse_posargs, {
    reports_t *reports = begin_reports();
    sources_t sources = { vector_new(4) };
    errors_t errors = { vector_new(4) };
    ap_t *ap = ap_new("test-argparse-posargs", NEW_VERSION(1, 0, 0));

    ap_event(ap, NULL, on_file, &sources);
    ap_error(ap, on_error, &errors);

    const char *argv[] = { "argparse-test", "file.txt", "lettuce.wad" };

    ap_parse(ap, reports, argv, 3);

    SHOULD_PASS("has 1 error", vector_len(errors.errors) == 1);
    SHOULD_PASS("has 1 file", vector_len(sources.files) == 1);
    SHOULD_PASS("file is file.txt", str_equal(vector_get(sources.files, 0), "file.txt"));
})

HARNESS("argparse", {
    ENTRY("posargs", test_argparse_posargs)
})
