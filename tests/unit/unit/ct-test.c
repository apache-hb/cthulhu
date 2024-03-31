#include "ct-test.h"

#include "core/macros.h"
#include "setup/setup2.h"
#include "setup/memory.h"
#include "memory/memory.h"

#include "format/backtrace.h"
#include "format/colour.h"

#include "io/console.h"
#include "io/io.h"

#include "arena/arena.h"

#include "std/str.h"
#include "os/core.h"

#include "base/log.h"

#include "backtrace/backtrace.h"

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
    source_info_t location;
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
static arena_t *gTestArena = NULL;

jmp_buf gPanicJump = { 0 };

static void test_panic_handler(source_info_t location, const char *fmt, va_list args)
{
    if (!gExpectingPanic)
    {
        io_t *io = io_stdout();

        io_printf(io, "unexpected panic [%s:%zu] => %s: ", location.file, location.line, location.function);
        io_vprintf(io, fmt, args);
        io_printf(io, "\n");

        bt_report_t *report = bt_report_collect(gTestArena);

        print_backtrace_t config = {
            .options = {
                .arena = gTestArena,
                .io = io,
                .pallete = &kColourDefault,
            },
            .header = eHeadingGeneric,
            .zero_indexed_lines = false,
        };

        print_backtrace(config, report);

        os_exit(CT_EXIT_INTERNAL);
    }

    char *msg = str_vformat(gTestArena, fmt, args);
    test_exception_t ex = { location, msg };
    gPanicException = ex;
    longjmp(gPanicJump, 1);
}

void test_install_panic_handler(void)
{
    ctu_log_update(true);
    bt_init();
    os_init();
    gPanicHandler = test_panic_handler;
    gTestArena = ctu_default_alloc();

    init_global_arena(gTestArena);
}

void test_install_electric_fence(void)
{
    init_global_arena(electric_fence_arena());
}

void test_begin_expect_panic(void)
{
    gExpectingPanic = true;
}

static void test_end_expect_panic(void)
{
    gExpectingPanic = false;
}

test_suite_t test_suite_new(const char *suite, arena_t *arena)
{
    test_suite_t s = {
        .suite_name = suite,
        .results = typevec_new(sizeof(test_result_t), 64, arena)
    };

    ARENA_IDENTIFY(s.results, "test results", &s, arena);

    return s;
}

int test_suite_finish(test_suite_t *suite)
{
    io_t *io = io_stdout();
    io_printf(io, "test suite %s\n", suite->suite_name);
    io_printf(io, "status, name, message, exception\n");
    int result = 0;

    test_result_t res;
    for (size_t i = 0; i < typevec_len(suite->results); i++)
    {
        typevec_get(suite->results, i, &res);
        io_printf(io, "%s, %s, %s", get_test_result_id(res.result), res.group_name, res.test_msg);
        if (res.has_exception)
        {
            io_printf(io, ", %s", res.ex.msg);
        }
        io_printf(io, "\n");
        int code = get_test_result_code(res.result);
        result = (code > result) ? code : result;
    }
    io_printf(io, "test suite %s %s (exit %d)\n", suite->suite_name, (result == 0) ? "PASSED" : "FAILED", result);
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

static void add_test_result(test_group_t *group, test_result_t result)
{
    typevec_push(group->suite->results, &result);
    test_end_expect_panic();
}

void group_notify_success(test_group_t *group, const char *msg)
{
    test_result_t result = test_result_status(group->name, msg, eTestPassed);
    add_test_result(group, result);
}

void group_notify_failure(test_group_t *group, const char *msg)
{
    test_result_t result = test_result_status(group->name, msg, eTestFailed);
    add_test_result(group, result);
}

void group_notify_exception(test_group_t *group, const char *msg)
{
    test_result_t result = test_result_exception(group->name, msg, gPanicException);
    add_test_result(group, result);
}
