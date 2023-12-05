#pragma once

#include "std/typed/vector.h"

#include <setjmp.h>

struct test_suite_t;
struct test_group_t;

extern jmp_buf gPanicJump;

typedef struct test_suite_t
{
    /// the name of the test suite
    const char *suite_name;

    typevec_t *results;
} test_suite_t;

/// @brief initialize the test suite
///
/// @param suite the name of the test suite
///
/// @return the test suite
test_suite_t test_suite_new(const char *suite);

/// @brief notify the suite that a test result has been received
///
/// @param group the test group
/// @param result the result of the test
/// @param msg the message to display
void group_notify_result(test_group_t *group, bool result, const char *msg);

/// @brief notify that a test failed
///
/// @param group the test group
/// @param msg the message to display
void group_notify_success(test_group_t *group, const char *msg);

/// @brief notify that a test failed
///
/// @param group the test group
/// @param msg the message to display
void group_notify_failure(test_group_t *group, const char *msg);

/// @brief notify that an exception occurred
///
/// @param group the test group
/// @param msg the message to display
void group_notify_exception(test_group_t *group, const char *msg);

/// @brief install the global panic handler
void test_install_panic_handler();

/// @brief begin expecting a panic
void test_begin_expect_panic();

/// @brief create a new test group
///
/// @param suite the test suite
/// @param name the name of the test group
///
/// @return the test case builder
test_group_t test_group(test_suite_t *suite, const char *name);

/// @brief finish a test suite and print the results
///
/// @param suite the test suite
///
/// @return the exit code of the test suite
int test_suite_finish(test_suite_t *suite);

typedef struct test_group_t
{
    test_suite_t *suite;
    const char *name;
} test_group_t;

/// @brief run a test that should pass and not throw an exception
/// @param msg the message to display
/// @param func the function to run
template<typename F>
void group_will_pass(test_group_t *group, const char *msg, F&& func)
{
    test_begin_expect_panic();
    if (setjmp(gPanicJump))
    {
        group_notify_exception(group, msg);
        return;
    }

    bool result = func();
    group_notify_result(group, result, msg);
}

/// @brief run a test that should fail, but not throw an exception
/// @param msg the message to display
/// @param func the function to run
template<typename F>
void group_will_fail(test_group_t *group, const char *msg, F&& func)
{
    test_begin_expect_panic();
    if (setjmp(gPanicJump))
    {
        group_notify_exception(group, msg);
        return;
    }

    bool result = func();
    group_notify_result(group, !result, msg);
}

#define GROUP_EXPECT_PASS(GROUP, ID, ...) \
    do { \
        group_will_pass(&GROUP, ID, [&] { return __VA_ARGS__; }); \
    } while (0)

#define GROUP_EXPECT_FAIL(GROUP, ID, ...) \
    do { \
        group_will_fail(&GROUP, ID, [&] { return __VA_ARGS__; }); \
    } while (0)

#define GROUP_EXPECT_PANIC(GROUP, ID, ...) \
    do { \
        test_begin_expect_panic(); \
        if (setjmp(gPanicJump)) \
        { \
            group_notify_success(&GROUP, ID); \
            break; \
        } \
        __VA_ARGS__; \
        group_notify_failure(&GROUP, ID); \
    } while (0)
