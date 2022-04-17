#pragma once

#include <stddef.h>
#include <stdio.h>

typedef enum {
    TEST_PASSED,
    TEST_FAILED,
    TEST_SKIPPED,
    TEST_ERROR,

    TEST_RESULT_TOTAL
} test_result_t;

static const char *RESULTS[TEST_RESULT_TOTAL] = {
    [TEST_PASSED] = "PASSED",
    [TEST_FAILED] = "FAILED",
    [TEST_SKIPPED] = "SKIPPED",
    [TEST_ERROR] = "ERROR",
};

static int CODES[TEST_RESULT_TOTAL] = {
    [TEST_PASSED] = 0,
    [TEST_FAILED] = 1,
    [TEST_SKIPPED] = 77,
    [TEST_ERROR] = 99,
};

typedef struct {
    const char *name;
    test_result_t(*test)(void);
} test_t;

#define TEST(name, ...) static test_result_t name(void) { __VA_ARGS__; return TEST_PASSED; }
#define TEST_CASE(msg, expr) do { if (!(expr)) { printf("%s:%d: %s\n%s\n", __FILE__, __LINE__, #expr, msg); return TEST_FAILED; } } while (0)

#define SHOULD_PASS(msg, expr) TEST_CASE(msg, expr)
#define SHOULD_FAIL(msg, expr) TEST_CASE(msg, !(expr))

#define ENTRY(name, func) (test_t){name, func}

static int run_tests(const char *suite, const test_t *tests, size_t total) {
    test_result_t result = TEST_PASSED;
    
    for (size_t i = 0; i < total; i++) {
        test_t test = tests[i];
        test_result_t res = test.test();
        result = (res == TEST_PASSED) ? result : res;
        printf("test %s %s\n", tests[i].name, RESULTS[res]);
    }

    printf("suite %s %s\n", suite, RESULTS[result]);
    return CODES[result];
}

#define HARNESS(name, ...) \
    static test_t TESTS[] = __VA_ARGS__; \
    int main() { \
        return run_tests(name, TESTS, sizeof(TESTS) / sizeof(test_t)); \
    }
