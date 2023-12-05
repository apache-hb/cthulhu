#pragma once

#include "base/panic.h"

#include <vector>
#include <setjmp.h>

class test_suite_t;
class test_group_t;

enum test_error_t
{
    eTestPassed,
    eTestFailed,
    eTestSkipped,
    eTestException,

    eTestTotal
};

struct test_exception_t
{
    panic_t panic;
    char *msg;
};

struct test_result_t
{
    /// the name of the test case that generated the result
    const char *group_name;

    /// test case message
    const char *test_msg;

    /// the result of the test
    test_error_t result;

    /// exception information
    bool has_exception;
    test_exception_t ex{};

    test_result_t(const char *group_name, const char *test_msg, test_error_t result)
        : group_name(group_name)
        , test_msg(test_msg)
        , result(result)
        , has_exception(false)
    { }

    test_result_t(const char *group_name, const char *test_msg, test_exception_t ex)
        : group_name(group_name)
        , test_msg(test_msg)
        , result(eTestException)
        , has_exception(true)
        , ex(ex)
    { }
};

class test_suite_t
{
    friend class test_group_t;

    /// @brief notify the suite that a test result has been received
    /// @param result the result of the test
    /// @param case_name the name of the test case
    /// @param msg the message to display
    void notify_result(bool result, const char *case_name, const char *msg);

    /// @brief notify that a test failed
    /// @param case_name the name of the test case
    /// @param msg the message to display
    void notify_success(const char *case_name, const char *msg);

    /// @brief notify that a test failed
    /// @param case_name the name of the test case
    /// @param msg the message to display
    void notify_failure(const char *case_name, const char *msg);

    /// @brief notify that an exception occurred
    /// @param ex the exception
    /// @param case_name the name of the test case
    /// @param msg the message to display
    void notify_exception(test_exception_t ex, const char *case_name, const char *msg);

    /// @brief add a result to the test suite
    /// @param case_name the name of the test case
    /// @param msg the message to display
    /// @param result the result of the test
    void add_result(test_result_t result);

    /// the name of the test suite
    const char *m_suite;

    /// the pending results of the test suite
    std::vector<test_result_t> m_results;
public:
    /// @brief install the global panic handler
    static void install_panic_handler();

    /// @brief begin expecting a panic
    static void begin_expect_panic();

    /// @brief end expecting a panic
    static void end_expect_panic();

    static jmp_buf gPanicJump;
    static test_exception_t gPanicException;

    /// @brief initialize the test suite
    /// @param suite the name of the test suite
    test_suite_t(const char *suite)
        : m_suite(suite)
    { }

    /// @brief finish the test suite and print the results
    /// @return the exit code
    int finish_suite();

    /// @brief create a new test group
    /// @param name the name of the test group
    /// @return the test case builder
    test_group_t test_group(const char *name);
};

class test_group_t
{
    friend class test_suite_t;

    test_suite_t& suite;
    const char *name;

    test_group_t(test_suite_t& suite, const char *name)
        : suite(suite)
        , name(name)
    { }

public:
    /// @brief run a test that should pass and not throw an exception
    /// @param msg the message to display
    /// @param func the function to run
    template<typename F>
    test_group_t& will_pass(const char *msg, F&& func)
    {
        static_assert(std::is_same<decltype(func()), bool>::value, "test function must return bool");

        test_suite_t::begin_expect_panic();
        if (setjmp(test_suite_t::gPanicJump))
        {
            suite.notify_exception(test_suite_t::gPanicException, name, msg);
            test_suite_t::end_expect_panic();
            return *this;
        }

        bool result = func();
        suite.notify_result(result, name, msg);

        test_suite_t::end_expect_panic();

        return *this;
    }

    /// @brief run a test that should fail, but not throw an exception
    /// @param msg the message to display
    /// @param func the function to run
    template<typename F>
    test_group_t& will_fail(const char *msg, F&& func)
    {
        static_assert(std::is_same<decltype(func()), bool>::value, "test function must return bool");

        test_suite_t::begin_expect_panic();
        if (setjmp(test_suite_t::gPanicJump))
        {
            suite.notify_exception(test_suite_t::gPanicException, name, msg);
            test_suite_t::end_expect_panic();
            return *this;
        }

        bool result = func();
        suite.notify_result(!result, name, msg);

        test_suite_t::end_expect_panic();

        return *this;
    }

    /// @brief run a test that should throw an exception
    /// @param msg the message to display
    /// @param func the function to run
    template<typename F>
    test_group_t& will_panic(const char *msg, F&& func)
    {
        test_suite_t::begin_expect_panic();

        if (setjmp(test_suite_t::gPanicJump))
        {
            suite.notify_success(name, msg);
            test_suite_t::end_expect_panic();
            return *this;
        }

        func();
        suite.notify_failure(name, msg);

        test_suite_t::end_expect_panic();

        return *this;
    }
};

#define EXPECT_PASS(ID, FN) will_pass(ID, [&] { return FN; })
#define EXPECT_FAIL(ID, FN) will_fail(ID, [&] { return FN; })
#define EXPECT_PANIC(ID, FN) will_panic(ID, [&] { FN; })
