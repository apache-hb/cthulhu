#include "ct-test.hpp"

#include <stdio.h>

#include "stacktrace/stacktrace.h"

#include "base/panic.h"
#include "std/str.h"

#include <stdlib.h>

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

static bool gExpectingPanic = false;

static void test_panic_handler(panic_t panic, const char *fmt, va_list args)
{
    if (!gExpectingPanic)
    {
        stacktrace_print(stdout);

        (void)printf("unexpected panic: %s:%zu: %s: ", panic.file, panic.line, panic.function);
        (void)vprintf(fmt, args);
        (void)printf("\n");
        abort();
    }

    char *msg = vformat(fmt, args);
    test_exception_t ex = { panic, msg };
    throw test_exception_t{ ex };
}

void test_suite_t::install_panic_handler()
{
    stacktrace_init();
    gPanicHandler = test_panic_handler;
}

void test_suite_t::begin_expect_panic()
{
    gExpectingPanic = true;
}

void test_suite_t::end_expect_panic()
{
    gExpectingPanic = false;
}

int test_suite_t::finish_suite()
{
    printf("test suite %s\n", m_suite);
    printf("status, name, message, exception\n");
    int result = 0;
    for (test_result_t res : m_results)
    {
        printf("%s, %s, %s", get_test_result_id(res.result), res.group_name, res.test_msg);
        if (res.has_exception)
        {
            printf(", %s", res.ex.msg);
        }
        printf("\n");
        int code = get_test_result_code(res.result);
        result = (code > result) ? code : result;
    }
    printf("test suite %s %s (exit %d)\n", m_suite, (result == 0) ? "PASSED" : "FAILED", result);
    return result;
}

test_group_t test_suite_t::test_group(const char *name)
{
    return test_group_t{ *this, name };
}

void test_suite_t::add_result(test_result_t result)
{
    m_results.push_back(result);
}

void test_suite_t::notify_result(bool result, const char *case_name, const char *msg)
{
    if (result)
    {
        notify_success(case_name, msg);
    }
    else
    {
        notify_failure(case_name, msg);
    }
}

void test_suite_t::notify_success(const char *case_name, const char *msg)
{
    test_result_t result{ case_name, msg, eTestPassed };
    add_result(result);
}

void test_suite_t::notify_failure(const char *case_name, const char *msg)
{
    test_result_t result{ case_name, msg, eTestFailed };
    add_result(result);
}

void test_suite_t::notify_exception(test_exception_t ex, const char *case_name, const char *msg)
{
    test_result_t result{ case_name, msg, ex };
    add_result(result);
}
