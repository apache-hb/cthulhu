#include "unit/ct-test.hpp"

#include "base/macros.h"
#include "base/memory.h"

#include "argparse/argparse.h"
#include "argparse/commands.h"

#include "report/report.h"

#include "std/vector.h"
#include "std/str.h"

#include <stdlib.h>

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
    CTU_UNUSED(ap);
    CTU_UNUSED(param);

    sources_t *sources = (sources_t*)data;
    const char *path = (char*)value;

    if (str_equal(path, "file.txt"))
    {
        vector_push(&sources->files, (char*)path);
        return eEventHandled;
    }

    return eEventContinue;
}

static ap_event_result_t on_error(ap_t *ap, const node_t *node, const char *message, void *data)
{
    CTU_UNUSED(ap);
    CTU_UNUSED(node);

    errors_t *errors = (errors_t*)data;
    vector_push(&errors->errors, (char*)message);
    return eEventHandled;
}

typedef struct error_stack_t
{
    int levels[5];
} error_stack_t;

typedef struct error_filter_t
{
    int level;
    error_stack_t *stack;
} error_filter_t;

static AP_EVENT(count_error, ap, node, message, data)
{
    CTU_UNUSED(ap);
    CTU_UNUSED(node);

    int i = strtol((char*)message, NULL, 10);
    error_filter_t *filter = (error_filter_t*)data;
    if (filter->level != i)
        return eEventContinue;

    error_stack_t *stack = filter->stack;
    stack->levels[i] += 1;
    return eEventHandled;
}

int main()
{
    test_suite_t::install_panic_handler();

    test_suite_t suite("argparse");

    // posargs
    {
        test_group_t group = suite.test_group("posargs");
        reports_t *reports = begin_reports();
        sources_t sources = { vector_new(4) };
        errors_t errors = { vector_new(4) };
        ap_t *ap = ap_new("test-argparse-posargs", NEW_VERSION(1, 0, 0));

        ap_event(ap, NULL, on_file, &sources);
        ap_error(ap, on_error, &errors);

        const char *argv[] = { "argparse-test", "file.txt", "lettuce.wad" };

        ap_parse(ap, reports, 3, argv);

        group.EXPECT_PASS("has 1 error", vector_len(errors.errors) == 1);
        group.EXPECT_PASS("has 1 file", vector_len(sources.files) == 1);
        group.EXPECT_PASS("file is file.txt", str_equal((char*)vector_get(sources.files, 0), "file.txt"));
    }

    // error stack
    {
        test_group_t group = suite.test_group("error stack");
        reports_t *reports = begin_reports();
        error_stack_t errors = { 0 };

        ap_t *ap = ap_new("test-argparse-error-stack", NEW_VERSION(1, 0, 0));

        error_filter_t f1 = { 1, &errors };
        error_filter_t f2 = { 2, &errors };
        error_filter_t f3 = { 3, &errors };
        error_filter_t f4 = { 4, &errors };

        ap_event(ap, NULL, count_error, &f1);
        ap_event(ap, NULL, count_error, &f2);
        ap_event(ap, NULL, count_error, &f3);
        ap_event(ap, NULL, count_error, &f4);

        const char *argv[] = { "argparse-test", "1", "2", "3", "4" };

        ap_parse(ap, reports, 5, argv);

        group.EXPECT_PASS("level 1 has 1 error", errors.levels[1] == 1);
        group.EXPECT_PASS("level 2 has 1 error", errors.levels[2] == 1);
        group.EXPECT_PASS("level 3 has 1 error", errors.levels[3] == 1);
        group.EXPECT_PASS("level 4 has 1 error", errors.levels[4] == 1);
    }

    return suite.finish_suite();
}
