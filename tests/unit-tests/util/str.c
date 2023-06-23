#include "std/str.h"
#include "std/vector.h"
#include "std/map.h"
#include "ct-test.h"

#include <string.h>
#include <stdint.h>

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
    SHOULD_FAIL("empty search", str_contains("", "string"));

    SHOULD_FAIL("no match", str_contains("string", "search"));
    SHOULD_PASS("smaller search", str_contains("string", "str"));
    SHOULD_PASS("equal strings", str_contains("string", "string"));
    SHOULD_FAIL("larger search", str_contains("string", "strings"));
})

static const char *kStringParts[] = {
    "zero", "one", "two", "three", "four", 
    "five", "six", "seven", "eight", "nine"
};
#define PARTS_SIZE (sizeof(kStringParts) / sizeof(const char *))

TEST(test_string_join, {
    vector_t *parts = vector_of(PARTS_SIZE);
    for (size_t i = 0; i < PARTS_SIZE; i++) {
        vector_set(parts, i, (char*)kStringParts[i]);
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
    const char *escaped;
    const char *unescaped;
} pair_t;

static pair_t kEscapes[] = {  
    { "\n", "\\n" },
    { "\r", "\\r" },
    { "\t", "\\t" },
    { "\v", "\\x0b" },
    { "\\", "\\\\" },
    { "\'", "\\\'" },
    { "\"", "\\\"" },
};

#define ESCAPE_SIZE (sizeof(kEscapes) / sizeof(pair_t))

TEST(test_string_normalize, {
    char *normalized = str_normalize("hello world");
    SHOULD_PASS("normalized equals", str_equal(normalized, "hello world"));

    char *newline = str_normalize("hello\nworld");
    SHOULD_PASS("newline equals", str_equal(newline, "hello\\nworld"));

    for (size_t i = 0; i < ESCAPE_SIZE; i++) {
        pair_t *escape = &kEscapes[i];
        char *input = format("hello %s world", escape->escaped);
        char *expected = format("hello %s world", escape->unescaped);

        char *result = str_normalize(input);

        char *name = format("escaped %s equal (`%s` != `%s`)", escape->unescaped, expected, result);
        SHOULD_PASS(name, str_equal(expected, result));
    }
})

TEST(test_string_normalizen, {
    char *shorter = str_normalizen("hello world", 5);
    SHOULD_PASS("normalized equals", str_equal(shorter, "hello"));

    char *normalized = str_normalizen("hello world", 12);
    SHOULD_PASS("normalized equals", str_equal(normalized, "hello world\\0"));

    char *newline = str_normalizen("hello\nworld", 12);
    SHOULD_PASS("newline equals", str_equal(newline, "hello\\nworld\\0"));

    for (size_t i = 0; i < ESCAPE_SIZE; i++) {
        pair_t *escape = &kEscapes[i];
        char *input = format("hello %s world", escape->escaped);
        char *expected = format("hello %s world", escape->unescaped);

        char *result = str_normalizen(input, strlen(input));

        char *name = format("escaped %s equal (`%s` != `%s`)", escape->unescaped, expected, result);
        SHOULD_PASS(name, str_equal(expected, result));
    }
})

TEST(test_string_split, {
    vector_t *hello = str_split("hello world", " ");
    SHOULD_PASS("length is 2", vector_len(hello) == 2);
    SHOULD_PASS("first is hello", str_equal(vector_get(hello, 0), "hello"));
    SHOULD_PASS("second is world", str_equal(vector_get(hello, 1), "world"));

    vector_t *nothing = str_split("three different words", "something");
    SHOULD_PASS("length is 1", vector_len(nothing) == 1);
    SHOULD_PASS("nothing contains one element", str_equal(vector_get(nothing, 0), "three different words"));

    vector_t *repeat = str_split("some,,,text", ",");
    SHOULD_PASS("length is 4", vector_len(repeat) == 4);
    SHOULD_PASS("first is some", str_equal(vector_get(repeat, 0), "some"));
    SHOULD_PASS("second is empty", str_equal(vector_get(repeat, 1), ""));
    SHOULD_PASS("third is empty", str_equal(vector_get(repeat, 2), ""));
    SHOULD_PASS("fourth is text", str_equal(vector_get(repeat, 3), "text"));

    vector_t *end = str_split("some text,,", ",");
    SHOULD_PASS("length is 3", vector_len(end) == 3);
    SHOULD_PASS("first is some text", str_equal(vector_get(end, 0), "some text"));
    SHOULD_PASS("second is empty", str_equal(vector_get(end, 1), ""));
    SHOULD_PASS("third is empty", str_equal(vector_get(end, 2), ""));

    vector_t *parts = str_split("a-path", "/");
    SHOULD_PASS("length is 1", vector_len(parts) == 1);
    SHOULD_PASS("first is a-path", str_equal(vector_get(parts, 0), "a-path"));
})

TEST(test_string_common_prefix, {
    vector_t *one_arg = vector_init((char*)"hello");
    const char *one_prefix = common_prefix(one_arg);
    SHOULD_PASS("one arg prefix", str_equal(one_prefix, "hello"));

    vector_t *no_common = vector_of(2);
    vector_set(no_common, 0, "hello");
    vector_set(no_common, 1, "world/world2");

    const char *no_common_prefix = common_prefix(no_common);
    SHOULD_PASS("no common prefix", str_equal(no_common_prefix, ""));

    vector_t *common = vector_of(2);
    vector_set(common, 0, "hello" NATIVE_PATH_SEPARATOR "stuff");
    vector_set(common, 1, "hello" NATIVE_PATH_SEPARATOR" world");

    const char *some_prefix = common_prefix(common);
    SHOULD_PASS("common prefix", str_equal(some_prefix, "hello" NATIVE_PATH_SEPARATOR));
})

TEST(test_string_rfind, {
    SHOULD_PASS("rfind no match", str_rfind("hello", "world") == SIZE_MAX);
    SHOULD_PASS("rfind match", str_rfind("hello world", "world") == 6);
    SHOULD_PASS("rfind match last", str_rfind("hello hello hello", "hello") == 12);
    SHOULD_PASS("rfind match last with bits", str_rfind("hello hello world", "hello") == 6);
})

TEST(test_string_equal, {
    SHOULD_PASS("equal", str_equal("hello", "hello"));
    SHOULD_FAIL("not equal", str_equal("hello", "world"));
})

TEST(test_string_replace, {
    char *nothing = str_replace("hello", "world", "");
    SHOULD_PASS("replace nothing", str_equal(nothing, "hello"));

    char *hello = str_replace("hello", "hello", "world");
    SHOULD_PASS("replace hello", str_equal(hello, "world"));

    char *newlines = str_replace("hello\nworld", "\n", "world");
    SHOULD_PASS("replace newlines", str_equal(newlines, "helloworldworld"));
})

TEST(test_string_replace_many, {
    map_t *entries = map_optimal(64);
    map_set(entries, "hello", "world");
    map_set(entries, "world", "hello");
    map_set(entries, "!", "?");
    map_set(entries, " ", "___");

    char *result = str_replace_many("hello world!", entries);
    SHOULD_PASS("replace all correctly", str_equal(result, "world___hello?"));
})

HARNESS("strings", {
    ENTRY("string str_startswith", test_string_startswith),
    ENTRY("string str_endswith", test_string_endswith),
    ENTRY("string contains", test_string_contains),
    ENTRY("string join", test_string_join),
    ENTRY("string repeat", test_string_repeat),
    ENTRY("string normalize", test_string_normalize),
    ENTRY("string normalizen", test_string_normalizen),
    ENTRY("string split", test_string_split),
    ENTRY("string common_prefix", test_string_common_prefix),
    ENTRY("string rfind", test_string_rfind),
    ENTRY("string equal", test_string_equal),
    ENTRY("string replace", test_string_replace),
    ENTRY("string replace_many", test_string_replace_many)
})
