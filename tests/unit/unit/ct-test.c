#include "ct-test.h"

#include "memory/memory.h"
#include "stacktrace/stacktrace.h"

#include "std/str.h"

#include <stdio.h>
#include <stdlib.h>

typedef enum test_error_t
{
    eTestPassed,
    eTestFailed,
    eTestSkipped,
    eTestException,

    eTestTotal
} test_error_t;

typedef struct test_exception_t
{
    panic_t panic;
    char *msg;
} test_exception_t;

typedef struct test_result_t
{
    /// the name of the test case that generated the result
    const char *group_name;

    /// test case message
    const char *test_msg;

    /// the result of the test
    test_error_t result;

    /// exception information
    bool has_exception;
    test_exception_t ex;
} test_result_t;

static const char *get_test_result_id(test_error_t result)
{
    switch (result)
    {
    case eTestPassed:       return "PASS";
    case eTestFailed:       return "FAIL";
    case eTestSkipped:      return "SKIP";
    case eTestException:    return "PANIC";
    default:                return "UNKNOWN";
    }
}

static int get_test_result_code(test_error_t result)
{
    switch (result)
    {
    case eTestPassed:       return 0;
    case eTestFailed:       return 1;
    case eTestSkipped:      return 77;
    case eTestException:    return 99;
    default:                return 1;
    }
}

/// test result

static test_result_t test_result_status(const char *group_name, const char *test_msg, test_error_t result)
{
    test_result_t res = {
        .group_name = group_name,
        .test_msg = test_msg,
        .result = result,
        .has_exception = false
    };

    return res;
}

static test_result_t test_result_exception(const char *group_name, const char *test_msg, test_exception_t ex)
{
    test_result_t res = {
        .group_name = group_name,
        .test_msg = test_msg,
        .result = eTestException,
        .has_exception = true,
        .ex = ex
    };

    return res;
}

static bool gExpectingPanic = false;
static test_exception_t gPanicException = { 0 };

jmp_buf gPanicJump = { 0 };

static void test_panic_handler(panic_t panic, const char *fmt, va_list args)
{
    if (!gExpectingPanic)
    {
        bt_print_trace(stdout);

        (void)printf("unexpected panic: %s:%zu: %s: ", panic.file, panic.line, panic.function);
        (void)vprintf(fmt, args);
        (void)printf("\n");
        abort();
    }

    char *msg = vformat(fmt, args);
    test_exception_t ex = { panic, msg };
    gPanicException = ex;
    longjmp(gPanicJump, 1);
}

void test_install_panic_handler()
{
    bt_init();
    gPanicHandler = test_panic_handler;

    init_global_arena(ctu_default_alloc());
}

void test_begin_expect_panic()
{
    gExpectingPanic = true;
}

static void test_end_expect_panic()
{
    gExpectingPanic = false;
}

test_suite_t test_suite_new(const char *suite)
{
    test_suite_t s = {
        .suite_name = suite,
        .results = typevec_new(sizeof(test_result_t), 64, ctu_default_alloc())
    };

    return s;
}

int test_suite_finish(test_suite_t *suite)
{
    printf("test suite %s\n", suite->suite_name);
    printf("status, name, message, exception\n");
    int result = 0;

    test_result_t res;
    for (size_t i = 0; i < typevec_len(suite->results); i++)
    {
        typevec_get(suite->results, i, &res);
        printf("%s, %s, %s", get_test_result_id(res.result), res.group_name, res.test_msg);
        if (res.has_exception)
        {
            printf(", %s", res.ex.msg);
        }
        printf("\n");
        int code = get_test_result_code(res.result);
        result = (code > result) ? code : result;
    }
    printf("test suite %s %s (exit %d)\n", suite->suite_name, (result == 0) ? "PASSED" : "FAILED", result);
    return result;
}

test_group_t test_group(test_suite_t *suite, const char *name)
{
    test_group_t group = {
        .suite = suite,
        .name = name
    };
    return group;
}

void group_notify_result(test_group_t *group, bool result, const char *msg)
{
    if (result)
    {
        group_notify_success(group, msg);
    }
    else
    {
        group_notify_failure(group, msg);
    }
}

void group_notify_success(test_group_t *group, const char *msg)
{
    test_result_t result = test_result_status(group->name, msg, eTestPassed);
    typevec_push(group->suite->results, &result);
    test_end_expect_panic();
}

void group_notify_failure(test_group_t *group, const char *msg)
{
    test_result_t result = test_result_status(group->name, msg, eTestFailed);
    typevec_push(group->suite->results, &result);
    test_end_expect_panic();
}

void group_notify_exception(test_group_t *group, const char *msg)
{
    test_result_t result = test_result_exception(group->name, msg, gPanicException);
    typevec_push(group->suite->results, &result);
    test_end_expect_panic();
}
