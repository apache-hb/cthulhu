#include "unit/ct-test.hpp"

#include "std/str.h"
#include "std/map.h"
#include "std/vector.h"

typedef struct {
    const char *escaped;
    const char *unescaped;
} pair_t;

static pair_t const kEscapes[] = {
    { "\n", "\\n" },
    { "\r", "\\r" },
    { "\t", "\\t" },
    { "\v", "\\x0b" },
    { "\\", "\\\\" },
    { "\'", "\\\'" },
    { "\"", "\\\"" },
};

static const size_t kEscapesSize = std::size(kEscapes);

static const char *const kStringParts[] = {
    "zero", "one", "two", "three", "four",
    "five", "six", "seven", "eight", "nine"
};

static const size_t kPartsSize = std::size(kStringParts);

int main()
{
    test_suite_t::install_panic_handler();

    test_suite_t suite("string");

    suite.test_group("str_equals")
        .EXPECT_PASS("empty strings", str_equal("", ""))
        .EXPECT_FAIL("empty first string", str_equal("", "string"))
        .EXPECT_FAIL("empty second string", str_equal("string", ""))
        .EXPECT_PASS("same string", str_equal("string", "string"))
        .EXPECT_FAIL("different string", str_equal("string", "different"))

        .EXPECT_PANIC("null lhs", str_equal(NULL, "string"))
        .EXPECT_PANIC("null rhs", str_equal("string", NULL))
        .EXPECT_PANIC("null both", str_equal(NULL, NULL));

    suite.test_group("str_startswith")
        .EXPECT_PASS("empty strings", str_startswith("", ""))
        .EXPECT_FAIL("empty string", str_startswith("", "string"))
        .EXPECT_PASS("shorter prefix", str_startswith("string", "str"))
        .EXPECT_PASS("same string", str_startswith("string", "string"))
        .EXPECT_FAIL("longer prefix", str_startswith("str", "string"))

        .EXPECT_PANIC("null lhs", str_startswith(NULL, "string"))
        .EXPECT_PANIC("null rhs", str_startswith("string", NULL))
        .EXPECT_PANIC("null both", str_startswith(NULL, NULL));

    suite.test_group("str_endswith")
        .EXPECT_PASS("empty strings", str_endswith("", ""))
        .EXPECT_FAIL("empty string", str_endswith("", "string"))
        .EXPECT_PASS("shorter suffix", str_endswith("string", "ing"))
        .EXPECT_PASS("same string", str_endswith("string", "string"))
        .EXPECT_FAIL("longer suffix", str_endswith("ing", "string"))

        .EXPECT_PANIC("null lhs", str_endswith(NULL, "string"))
        .EXPECT_PANIC("null rhs", str_endswith("string", NULL))
        .EXPECT_PANIC("null both", str_endswith(NULL, NULL));

    suite.test_group("str_contains")
        .EXPECT_PASS("empty strings", str_contains("", ""))
        .EXPECT_FAIL("empty string", str_contains("", "string"))
        .EXPECT_PASS("shorter suffix", str_contains("string", "ing"))
        .EXPECT_PASS("same string", str_contains("string", "string"))
        .EXPECT_FAIL("longer suffix", str_contains("ing", "string"))

        .EXPECT_PANIC("null lhs", str_contains(NULL, "string"))
        .EXPECT_PANIC("null rhs", str_contains("string", NULL))
        .EXPECT_PANIC("null both", str_contains(NULL, NULL));

    suite.test_group("str_rfind")
        .EXPECT_PASS("no match", str_rfind("hello world!", "universe") == SIZE_MAX)
        .EXPECT_PASS("match", str_rfind("hello world!", "world") == 6)
        .EXPECT_PASS("match last", str_rfind("hello hello hello", "hello") == 12)
        .EXPECT_PASS("find last with bits", str_rfind("hello hello world", "hello") == 6)
        .EXPECT_PANIC("empty search", str_rfind("hello", ""))
        .EXPECT_PANIC("null search", str_rfind("hello", NULL))
        .EXPECT_PANIC("null str", str_rfind(NULL, "hello"))
        .EXPECT_PANIC("null all", str_rfind(NULL, NULL));

    suite.test_group("str_replace")
        .will_pass("replace one", [] {
            char *result = str_replace("hello world!", "world", "universe");
            return str_equal(result, "hello universe!");
        })
        .will_pass("replace none", [] {
            char *result = str_replace("hello world!", "universe", "world");
            return str_equal(result, "hello world!");
        })
        .will_pass("replace empty", [] {
            char *result = str_replace("hello world!", "", "universe");
            return str_equal(result, "hello world!");
        })
        .EXPECT_PANIC("null str", str_replace(NULL, "world", "universe"))
        .EXPECT_PANIC("null old", str_replace("hello world!", NULL, "universe"))
        .EXPECT_PANIC("null new", str_replace("hello world!", "world", NULL))
        .EXPECT_PANIC("null all", str_replace(NULL, NULL, NULL));

    suite.test_group("str_replace_many")
        .will_pass("replace many", [] {
            map_t *entries = map_optimal(64);
            map_set(entries, "hello", (void*)"world");
            map_set(entries, "world", (void*)"hello");
            map_set(entries, "!", (void*)"?");
            map_set(entries, " ", (void*)"___");

            char *result = str_replace_many("hello world!", entries);
            return str_equal(result, "world___hello?");
        })
        .will_panic("null entry", [] {
            map_t *entries = map_optimal(64);
            map_set(entries, "hello", NULL);

            str_replace_many("hello world!", entries);
        })
        .EXPECT_PANIC("null str", str_replace_many(NULL, map_new(1)))
        .EXPECT_PANIC("null map", str_replace_many("hello world!", NULL))
        .EXPECT_PANIC("null all", str_replace_many(NULL, NULL));

    vector_t *parts = vector_of(kPartsSize);
    for (size_t i = 0; i < kPartsSize; i++) {
        vector_set(parts, i, (char*)kStringParts[i]);
    }

    suite.test_group("str_join")
        .will_pass("joined equals", [parts] {
            char *joined = str_join(" ", parts);
            return str_equal(joined, "zero one two three four five six seven eight nine");
        })
        .will_pass("joined empty", [] {
            vector_t *empty = vector_of(0);
            char *joined = str_join(" ", empty);
            return str_equal(joined, "");
        })
        .will_pass("joined one", [] {
            vector_t *one = vector_init((char*)"hello");
            char *joined = str_join(" ", one);
            return str_equal(joined, "hello");
        })
        .EXPECT_PANIC("null sep", str_join(NULL, vector_of(0)))
        .EXPECT_PANIC("null parts", str_join(" ", NULL))
        .EXPECT_PANIC("null item", str_join(" ", vector_init(NULL)))
        .EXPECT_PANIC("null all", str_join(NULL, NULL));

    suite.test_group("str_repeat")
        .will_pass("repeat equals", [] {
            char *repeated = str_repeat("hello", 3);
            return str_equal(repeated, "hellohellohello");
        })
        .will_pass("repeat empty", [] {
            char *repeated = str_repeat("", 3);
            return str_equal(repeated, "");
        })
        .will_pass("repeat none", [] {
            char *repeated = str_repeat("hello", 0);
            return str_equal(repeated, "");
        })
        .will_pass("repeat one", [] {
            char *repeated = str_repeat("hello", 1);
            return str_equal(repeated, "hello");
        });

    ///
    /// str split
    ///

    {
        vector_t *hello = str_split("hello world!", " ");
        suite.test_group("str_split equals")
            .EXPECT_PASS("length", vector_len(hello) == 2)
            .EXPECT_PASS("first", str_equal((char*)vector_get(hello, 0), "hello"))
            .EXPECT_PASS("second", str_equal((char*)vector_get(hello, 1), "world!"));
    }

    {
        vector_t *nothing = str_split("three different words", "something");
        suite.test_group("str_split nothing")
            .EXPECT_PASS("length", vector_len(nothing) == 1)
            .EXPECT_PASS("first", str_equal((char*)vector_get(nothing, 0), "three different words"));
    }

    {
        vector_t *repeat = str_split("some,,,text", ",");
        suite.test_group("str_split repeat")
            .EXPECT_PASS("length", vector_len(repeat) == 4)
            .EXPECT_PASS("first", str_equal((char*)vector_get(repeat, 0), "some"))
            .EXPECT_PASS("second", str_equal((char*)vector_get(repeat, 1), ""))
            .EXPECT_PASS("third", str_equal((char*)vector_get(repeat, 2), ""))
            .EXPECT_PASS("fourth", str_equal((char*)vector_get(repeat, 3), "text"));
    }

    {
        vector_t *path_parts = str_split("a-path", "/");
        suite.test_group("str_split parts")
            .EXPECT_PASS("length", vector_len(path_parts) == 1)
            .EXPECT_PASS("first", str_equal((char*)vector_get(path_parts, 0), "a-path"));
    }

    {
        vector_t *empty_string = str_split("", " ");
        suite.test_group("str_split empty string")
            .EXPECT_PASS("length", vector_len(empty_string) == 1)
            .EXPECT_PASS("first", str_equal((char*)vector_get(empty_string, 0), ""));
    }

    {
        const char *text = "hello world!";
        vector_t *empty_sep = str_split(text, "");
        test_group_t empty_split_group = suite.test_group("str_split empty sep")
            .EXPECT_PASS("length", vector_len(empty_sep) == 12);

        for (size_t i = 0; i < 12; i++)
        {
            char *part = (char*)vector_get(empty_sep, i);
            char expected[] = { text[i], '\0' };
            empty_split_group.EXPECT_PASS("part", str_equal(part, expected));
        }
    }

    ///
    /// str normalize
    ///

    test_group_t normalize_group = suite.test_group("str_normalize")
        .will_pass("normalize equals", [] {
            char *normalized = str_normalize("hello world!");
            return str_equal(normalized, "hello world!");
        })
        .will_pass("normalize empty", [] {
            char *normalized = str_normalize("");
            return str_equal(normalized, "");
        })
        .will_pass("normalize newline", [] {
            char *result = str_normalize("hello\n world!");
            return str_equal(result, "hello\\n world!");
        })
        .EXPECT_PANIC("normalize null", str_normalize(NULL));

    test_group_t normalizen_group = suite.test_group("str_normalizen")
        .will_pass("normalizen equals", [] {
            char *normalized = str_normalizen("hello world!", 12);
            return str_equal(normalized, "hello world!");
        })
        .will_pass("normalizen empty", [] {
            char *normalized = str_normalizen("", 0);
            return str_equal(normalized, "");
        })
        .EXPECT_PANIC("normalizen null", str_normalizen(NULL, 0));

    for (size_t i = 0; i < kEscapesSize; i++)
    {
        pair_t pair = kEscapes[i];

        char *input = format("hello %s world", pair.escaped);
        char *expected = format("hello %s world", pair.unescaped);

        char *result = str_normalize(input);
        char *result_n = str_normalizen(input, strlen(input));

        char *name = format("escaped %s equal (`%s` != `%s`)", pair.escaped, expected, result);
        normalize_group.EXPECT_PASS(name, str_equal(result, expected));
        normalizen_group.EXPECT_PASS(name, str_equal(result_n, expected));
    }

    ///
    /// common prefix
    ///

    vector_t *one_arg = vector_init((char*)"hello");
    const char *one_prefix = common_prefix(one_arg);

    vector_t *no_common = vector_of(2);
    vector_set(no_common, 0, (void*)"hello");
    vector_set(no_common, 1, (void*)"world/world2");

    const char *no_common_prefix = common_prefix(no_common);

    vector_t *common = vector_of(2);
    vector_set(common, 0, (void*)"hello" NATIVE_PATH_SEPARATOR "stuff");
    vector_set(common, 1, (void*)"hello" NATIVE_PATH_SEPARATOR" world");

    const char *some_prefix = common_prefix(common);

    suite.test_group("str_common_prefix")
        .EXPECT_PASS("one arg", str_equal(one_prefix, "hello"))
        .EXPECT_PASS("no common", str_equal(no_common_prefix, ""))
        .EXPECT_PASS("common", str_equal(some_prefix, "hello" NATIVE_PATH_SEPARATOR));

    return suite.finish_suite();
}
