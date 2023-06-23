#pragma once

#include <stddef.h>
#include <stdio.h>

// TODO: find a better way to init stuff, this currently means we depend on cthulhu
//       when the tests only need common
#include "cthulhu/mediator/interface.h"

#include "report/report.h"

typedef enum {
    eTestPassed,
    eTestFailed,
    eTestSkipped,
    eTestError,

    eTestTotal
} test_result_t;

static const char *kResultNames[eTestTotal] = {
    [eTestPassed] = "PASSED",
    [eTestFailed] = "FAILED",
    [eTestSkipped] = "SKIPPED",
    [eTestError] = "eFatal"
};

static int kErrorCodes[eTestTotal] = {
    [eTestPassed] = 0,
    [eTestFailed] = 1,
    [eTestSkipped] = 77,
    [eTestError] = 99
};

typedef struct {
    const char *name;
    test_result_t(*test)(void);
} test_t;

#define TEST(name, ...) static test_result_t name(void) { __VA_ARGS__; return eTestPassed; }
#define TEST_CASE(msg, expr) do { if (!(expr)) { printf("%s:%d: %s\n%s\n", __FILE__, __LINE__, #expr, msg); return eTestFailed; } } while (0)

#define SHOULD_PASS(msg, expr) TEST_CASE(msg, expr)
#define SHOULD_FAIL(msg, expr) TEST_CASE(msg, !(expr))

#define ENTRY(name, func) (test_t){name, func}

static int run_tests(const char *suite, const test_t *tests, size_t total) {
    const version_info_t version = {
        .license = "GPLv3",
        .desc = "unit test harness",
        .author = "Elliot",
        .version = NEW_VERSION(3, 0, 0)
    };

    verbose = true;

    mediator_new("test", version);

    test_result_t result = eTestPassed;
    
    for (size_t i = 0; i < total; i++) {
        test_t test = tests[i];
        test_result_t res = test.test();
        result = (res == eTestPassed) ? result : res;
        printf("test %s %s\n", tests[i].name, kResultNames[res]);
    }

    printf("suite %s %s\n", suite, kResultNames[result]);
    return kErrorCodes[result];
}

#define HARNESS(name, ...) \
    int main(void) { \
        test_t all[] = __VA_ARGS__; \
        return run_tests(name, all, sizeof(all) / sizeof(test_t)); \
    }
