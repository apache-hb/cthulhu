#pragma once

#include <stddef.h>

typedef enum {
    TEST_PASSED = 0,
    TEST_FAILED = 1,
    TEST_SKIPPED = 77,
    TEST_ERROR = 99,

    TEST_RESULT_TOTAL
} test_result_t;
