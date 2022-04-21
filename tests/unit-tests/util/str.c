#include "cthulhu/util/str.h"
#include "test.h"

TEST(test_string_startswith, {
    SHOULD_PASS("empty strings", str_startswith("", ""));
    SHOULD_FAIL("empty string", str_startswith("", "string"));

    SHOULD_PASS("shorter prefix", str_startswith("string", "str"));
    SHOULD_PASS("equal strings", str_startswith("string", "string"));
    SHOULD_FAIL("longer prefix", str_startswith("string", "strings"));
})

TEST(test_string_endswith, {
    SHOULD_PASS("empty strings", str_endswith("", ""));
    SHOULD_FAIL("empty string", str_endswith("", "string"));

    SHOULD_PASS("shorter suffix", str_endswith("string", "ng"));
    SHOULD_PASS("equal strings", str_endswith("string", "string"));
    SHOULD_FAIL("longer suffix", str_endswith("string", "strings"));
})

TEST(test_string_contains, {
    SHOULD_FAIL("empty strings", str_contains("", ""));
    SHOULD_FAIL("empty substring", str_contains("string", ""));
    SHOULD_FAIL("empty search", str_contains("", "string"));

    SHOULD_FAIL("no match", str_contains("string", "search"));
    SHOULD_PASS("smaller search", str_contains("string", "str"));
    SHOULD_PASS("equal strings", str_contains("string", "string"));
    SHOULD_FAIL("larger search", str_contains("string", "strings"));
})

static const char *PARTS[] = {
    "zero", "one", "two", "three", "four", 
    "five", "six", "seven", "eight", "nine"
};
#define PARTS_SIZE (sizeof(PARTS) / sizeof(const char *))

TEST(test_string_join, {
    vector_t *parts = vector_of(PARTS_SIZE);
    for (size_t i = 0; i < PARTS_SIZE; i++) {
        vector_set(parts, i, (char*)PARTS[i]);
    }
    char *joined_parts = str_join("-", parts);
    SHOULD_PASS("joined equals", str_equal(joined_parts, "zero-one-two-three-four-five-six-seven-eight-nine"));

    vector_t *empty = vector_new(4);
    char *joined_empty = str_join("-", empty);
    SHOULD_PASS("empty joined equals", str_equal(joined_empty, ""));

    vector_t *single = vector_init((char*)"hello");
    char *joined_single = str_join("-", single);
    SHOULD_PASS("single joined equals", str_equal(joined_single, "hello"));
})

TEST(test_string_repeat, {
    char *repeated = str_repeat("hello", 3);
    SHOULD_PASS("repeated equals", str_equal(repeated, "hellohellohello"));

    char *empty = str_repeat("", 3);
    SHOULD_PASS("empty equals", str_equal(empty, ""));

    char *none = str_repeat("hello", 0);
    SHOULD_PASS("none equals", str_equal(none, ""));
})

typedef struct {
    char escaped;
    const char *unescaped;
} pair_t;

static pair_t ESCAPES[] = {  
    { '\a', "\\x07" },
    { '\b', "\\x08" },
    { '\f', "\\x0c" },
    { '\n', "\\x0a" },
    { '\r', "\\x0d" },
    { '\t', "\\x09" },
    { '\v', "\\x0b" },
    { '\\', "\\\\" },
    { '\'', "\\\'" },
    { '\"', "\\\"" },
};

#define ESCAPE_SIZE (sizeof(ESCAPES) / sizeof(pair_t))

TEST(test_string_normalize, {
    char *normalized = str_normalize("hello world");
    SHOULD_PASS("normalized equals", str_equal(normalized, "hello world"));

    char *newline = str_normalize("hello\nworld");
    printf("%s\n", newline);
    SHOULD_PASS("newline equals", str_equal(newline, "hello\\x0aworld"));

    for (size_t i = 0; i < ESCAPE_SIZE; i++) {
        pair_t *escape = &ESCAPES[i];
        char *input = format("hello %c world", escape->escaped);
        char *expected = format("hello %s world", escape->unescaped);

        char *result = str_normalize(input);

        printf("%s\n", result);
        char *name = format("escaped %s equal", escape->unescaped);
        SHOULD_PASS(name, str_equal(expected, result));
    }
})

HARNESS("strings", {
    ENTRY("string str_startswith", test_string_startswith),
    ENTRY("string str_endswith", test_string_endswith),
    ENTRY("string contains", test_string_contains),
    ENTRY("string join", test_string_join),
    ENTRY("string repeat", test_string_repeat),
    ENTRY("string normalize", test_string_normalize)
})
