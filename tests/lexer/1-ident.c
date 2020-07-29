#include "framework/test.h"

MAKE_TEST("basic identifiers", {
    TestStream stream = testStream("a b c");

    LEXING_STATE(state, stream)
})