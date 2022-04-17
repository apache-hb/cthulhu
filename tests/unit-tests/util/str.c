#include "cthulhu/util/str.h"
#include "test.h"

TEST(test_string_startswith, {
    SHOULD_PASS("empty strings", startswith("", ""));
    SHOULD_FAIL("empty string", startswith("", "string"));

    SHOULD_PASS("shorter prefix", startswith("string", "str"));
    SHOULD_PASS("equal strings", startswith("string", "string"));
    SHOULD_FAIL("longer prefix", startswith("string", "strings"));
})

TEST(test_string_endswith, {
    SHOULD_PASS("empty strings", endswith("", ""));
    SHOULD_FAIL("empty string", endswith("", "string"));

    SHOULD_PASS("shorter suffix", endswith("string", "ng"));
    SHOULD_PASS("equal strings", endswith("string", "string"));
    SHOULD_FAIL("longer suffix", endswith("string", "strings"));
})

TEST(test_string_contains, {
    SHOULD_FAIL("empty strings", strcontains("", ""));
    SHOULD_FAIL("empty substring", strcontains("string", ""));
    SHOULD_FAIL("empty search", strcontains("", "string"));

    SHOULD_FAIL("no match", strcontains("string", "search"));
    SHOULD_PASS("smaller search", strcontains("string", "str"));
    SHOULD_PASS("equal strings", strcontains("string", "string"));
    SHOULD_FAIL("larger search", strcontains("string", "strings"));
})

HARNESS("strings", {
    ENTRY("string startswith", test_string_startswith),
    ENTRY("string endswith", test_string_endswith),
    ENTRY("string contains", test_string_contains)
})
