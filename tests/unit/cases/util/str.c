#include "memory/arena.h"
#include "unit/ct-test.h"

#include "defaults/memory.h"

#include "std/map.h"
#include "std/str.h"
#include "std/vector.h"

#include <string.h>
#include <stdint.h>

typedef struct pair_t
{
    const char *escaped;
    const char *unescaped;
} pair_t;

static pair_t kEscapes[] = {
    {"\n", "\\n"},
    {"\r", "\\r"},
    {"\t", "\\t"},
    {"\v", "\\x0b"},
    {"\\", "\\\\"},
    {"\'", "\\\'"},
    {"\"", "\\\""},
};

#define ESCAPES_LEN (sizeof(kEscapes) / sizeof(pair_t))

static const char *kStringParts[] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};

#define STRING_PARTS_LEN (sizeof(kStringParts) / sizeof(char *))

int main(void)
{
    test_install_panic_handler();

    arena_t *arena = ctu_default_alloc();
    test_suite_t suite = test_suite_new("string", arena);
    {
        test_group_t group = test_group(&suite, "str_equals");
        GROUP_EXPECT_PASS(group, "empty strings", str_equal("", ""));
        GROUP_EXPECT_FAIL(group, "empty first string", str_equal("", "string"));
        GROUP_EXPECT_FAIL(group, "empty second string", str_equal("string", ""));
        GROUP_EXPECT_PASS(group, "same string", str_equal("string", "string"));
        GROUP_EXPECT_FAIL(group, "different string", str_equal("string", "different"));

        GROUP_EXPECT_PANIC(group, "null lhs", (void)str_equal(NULL, "string"));
        GROUP_EXPECT_PANIC(group, "null rhs", (void)str_equal("string", NULL));
        GROUP_EXPECT_PANIC(group, "null both", (void)str_equal(NULL, NULL));
    }
    {
        test_group_t group = test_group(&suite, "str_startswith");
        GROUP_EXPECT_PASS(group, "empty strings", str_startswith("", ""));
        GROUP_EXPECT_FAIL(group, "empty string", str_startswith("", "string"));
        GROUP_EXPECT_PASS(group, "shorter prefix", str_startswith("string", "str"));
        GROUP_EXPECT_PASS(group, "same string", str_startswith("string", "string"));
        GROUP_EXPECT_FAIL(group, "longer prefix", str_startswith("str", "string"));

        GROUP_EXPECT_PANIC(group, "null lhs", (void)str_startswith(NULL, "string"));
        GROUP_EXPECT_PANIC(group, "null rhs", (void)str_startswith("string", NULL));
        GROUP_EXPECT_PANIC(group, "null both", (void)str_startswith(NULL, NULL));
    }
    {
        test_group_t group = test_group(&suite, "str_endswith");
        GROUP_EXPECT_PASS(group, "empty strings", str_endswith("", ""));
        GROUP_EXPECT_FAIL(group, "empty string", str_endswith("", "string"));
        GROUP_EXPECT_PASS(group, "shorter suffix", str_endswith("string", "ing"));
        GROUP_EXPECT_PASS(group, "same string", str_endswith("string", "string"));
        GROUP_EXPECT_FAIL(group, "longer suffix", str_endswith("ing", "string"));

        GROUP_EXPECT_PANIC(group, "null lhs", (void)str_endswith(NULL, "string"));
        GROUP_EXPECT_PANIC(group, "null rhs", (void)str_endswith("string", NULL));
        GROUP_EXPECT_PANIC(group, "null both", (void)str_endswith(NULL, NULL));
    }
    {
        test_group_t group = test_group(&suite, "str_contains");
        GROUP_EXPECT_PASS(group, "empty strings", str_contains("", ""));
        GROUP_EXPECT_FAIL(group, "empty string", str_contains("", "string"));
        GROUP_EXPECT_PASS(group, "shorter suffix", str_contains("string", "ing"));
        GROUP_EXPECT_PASS(group, "same string", str_contains("string", "string"));
        GROUP_EXPECT_FAIL(group, "longer suffix", str_contains("ing", "string"));

        GROUP_EXPECT_PANIC(group, "null lhs", (void)str_contains(NULL, "string"));
        GROUP_EXPECT_PANIC(group, "null rhs", (void)str_contains("string", NULL));
        GROUP_EXPECT_PANIC(group, "null both", (void)str_contains(NULL, NULL));
    }
    {
        test_group_t group = test_group(&suite, "str_rfind");
        GROUP_EXPECT_PASS(group, "no match", str_rfind("hello world!", "universe") == SIZE_MAX);
        GROUP_EXPECT_PASS(group, "match", str_rfind("hello world!", "world") == 6);
        GROUP_EXPECT_PASS(group, "match last", str_rfind("hello hello hello", "hello") == 12);
        GROUP_EXPECT_PASS(group, "find last with bits", str_rfind("hello hello world", "hello") == 6);
        GROUP_EXPECT_PANIC(group, "empty search", str_rfind("hello", ""));
        GROUP_EXPECT_PANIC(group, "null search", str_rfind("hello", NULL));
        GROUP_EXPECT_PANIC(group, "null str", str_rfind(NULL, "hello"));
        GROUP_EXPECT_PANIC(group, "null all", str_rfind(NULL, NULL));
    }
    {
        test_group_t group = test_group(&suite, "str_replace");
        GROUP_EXPECT_PASS2(group, "replace one", {
            char *result = str_replace("hello world!", "world", "universe", arena);
            CTASSERT(str_equal(result, "hello universe!"));
        });
        GROUP_EXPECT_PASS2(group, "replace none", {
            char *result = str_replace("hello world!", "universe", "world", arena);
            CTASSERT(str_equal(result, "hello world!"));
        });
        GROUP_EXPECT_PASS2(group, "replace empty", {
            char *result = str_replace("hello world!", "", "universe", arena);
            CTASSERT(str_equal(result, "hello world!"));
        });
        GROUP_EXPECT_PANIC(group, "null str", (void)str_replace(NULL, "world", "universe", arena));
        GROUP_EXPECT_PANIC(group, "null old", (void)str_replace("hello world!", NULL, "universe", arena));
        GROUP_EXPECT_PANIC(group, "null new", (void)str_replace("hello world!", "world", NULL, arena));
        GROUP_EXPECT_PANIC(group, "null all", (void)str_replace(NULL, NULL, NULL, arena));
    }
    {
        test_group_t group = test_group(&suite, "str_replace_many");
        GROUP_EXPECT_PASS2(group, "replace many", {
            map_t *entries = map_optimal_info(64, kTypeInfoString, arena);
            map_set_ex(entries, "hello", (void *)"world");
            map_set_ex(entries, "world", (void *)"hello");
            map_set_ex(entries, "!", (void *)"?");
            map_set_ex(entries, " ", (void *)"___");

            char *result = str_replace_many("hello world!", entries);
            CTASSERT(str_equal(result, "world___hello?"));
        });
        GROUP_EXPECT_PANIC(group, "null entry", {
            map_t *entries = map_optimal_info(64, kTypeInfoString, arena);
            map_set_ex(entries, "hello", NULL);

            (void)str_replace_many("hello world!", entries);
        });
        GROUP_EXPECT_PANIC(group, "null str", (void)str_replace_many(NULL, map_new_info(1, kTypeInfoString, arena)));
        GROUP_EXPECT_PANIC(group, "null map", (void)str_replace_many("hello world!", NULL));
        GROUP_EXPECT_PANIC(group, "null all", (void)str_replace_many(NULL, NULL));
    }

    vector_t *parts = vector_of_arena(STRING_PARTS_LEN, arena);
    for (size_t i = 0; i < STRING_PARTS_LEN; i++)
    {
        vector_set(parts, i, (char *)kStringParts[i]);
    }

    {
        test_group_t group = test_group(&suite, "str_join");
        GROUP_EXPECT_PASS2(group, "joined equals", {
            char *joined = str_join(" ", parts);
            CTASSERT(str_equal(joined, "zero one two three four five six seven eight nine"));
        });
        GROUP_EXPECT_PASS2(group, "joined empty", {
            vector_t *empty = vector_of_arena(0, arena);
            char *joined = str_join(" ", empty);
            CTASSERT(str_equal(joined, ""));
        });
        GROUP_EXPECT_PASS2(group, "joined one", {
            vector_t *one = vector_init_arena((char *)"hello", arena);
            char *joined = str_join(" ", one);
            CTASSERT(str_equal(joined, "hello"));
        });
        GROUP_EXPECT_PANIC(group, "null sep", (void)str_join(NULL, vector_of(0)));
        GROUP_EXPECT_PANIC(group, "null parts", (void)str_join(" ", NULL));
        GROUP_EXPECT_PANIC(group, "null item", (void)str_join(" ", vector_init(NULL)));
        GROUP_EXPECT_PANIC(group, "null all", (void)str_join(NULL, NULL));
    }
    {
        test_group_t group = test_group(&suite, "str_repeat");
        GROUP_EXPECT_PASS2(group, "repeat equals", {
            char *repeated = str_repeat("hello", 3);
            CTASSERT(str_equal(repeated, "hellohellohello"));
        });
        GROUP_EXPECT_PASS2(group, "repeat empty", {
            char *repeated = str_repeat("", 3);
            CTASSERT(str_equal(repeated, ""));
        });
        GROUP_EXPECT_PASS2(group, "repeat none", {
            char *repeated = str_repeat("hello", 0);
            CTASSERT(str_equal(repeated, ""));
        });
        GROUP_EXPECT_PASS2(group, "repeat one", {
            char *repeated = str_repeat("hello", 1);
            CTASSERT(str_equal(repeated, "hello"));
        });
    }
    ///
    /// str split
    ///

    {
        vector_t *hello = str_split_arena("hello world!", " ", arena);
        test_group_t group = test_group(&suite, "str_split equals");
        GROUP_EXPECT_PASS(group, "length", vector_len(hello) == 2);
        GROUP_EXPECT_PASS(group, "first", str_equal((char *)vector_get(hello, 0), "hello"));
        GROUP_EXPECT_PASS(group, "second", str_equal((char *)vector_get(hello, 1), "world!"));
    }

    {
        vector_t *nothing = str_split_arena("three different words", "something", arena);
        test_group_t group = test_group(&suite, "str_split nothing");
        GROUP_EXPECT_PASS(group, "length", vector_len(nothing) == 1);
        GROUP_EXPECT_PASS(group, "first", str_equal((char *)vector_get(nothing, 0), "three different words"));
    }

    {
        vector_t *repeat = str_split_arena("some,,,text", ",", arena);
        test_group_t group = test_group(&suite, "str_split repeat");
        GROUP_EXPECT_PASS(group, "length", vector_len(repeat) == 4);
        GROUP_EXPECT_PASS(group, "first", str_equal((char *)vector_get(repeat, 0), "some"));
        GROUP_EXPECT_PASS(group, "second", str_equal((char *)vector_get(repeat, 1), ""));
        GROUP_EXPECT_PASS(group, "third", str_equal((char *)vector_get(repeat, 2), ""));
        GROUP_EXPECT_PASS(group, "fourth", str_equal((char *)vector_get(repeat, 3), "text"));
    }

    {
        vector_t *path_parts = str_split_arena("a-path", "/", arena);
        test_group_t group = test_group(&suite, "str_split parts");
        GROUP_EXPECT_PASS(group, "length", vector_len(path_parts) == 1);
        GROUP_EXPECT_PASS(group, "first", str_equal((char *)vector_get(path_parts, 0), "a-path"));
    }

    {
        vector_t *empty_string = str_split_arena("", " ", arena);
        test_group_t group = test_group(&suite, "str_split empty string");
        GROUP_EXPECT_PASS(group, "length", vector_len(empty_string) == 1);
        GROUP_EXPECT_PASS(group, "first", str_equal((char *)vector_get(empty_string, 0), ""));
    }

    {
        const char *text = "hello world!";
        vector_t *empty_sep = str_split_arena(text, "", arena);
        test_group_t empty_split_group = test_group(&suite, "str_split empty sep");
        GROUP_EXPECT_PASS(empty_split_group, "length", vector_len(empty_sep) == 12);

        for (size_t i = 0; i < 12; i++)
        {
            char *part = (char *)vector_get(empty_sep, i);
            char expected[] = {text[i], '\0'};
            GROUP_EXPECT_PASS(empty_split_group, "part", str_equal(part, expected));
        }
    }

    ///
    /// str normalize
    ///

    test_group_t normalize_group = test_group(&suite, "str_normalize");
    GROUP_EXPECT_PASS2(normalize_group, "normalize equals", {
        char *normalized = str_normalize("hello world!");
        CTASSERT(str_equal(normalized, "hello world!"));
    });
    GROUP_EXPECT_PASS2(normalize_group, "normalize empty", {
        char *normalized = str_normalize("");
        CTASSERT(str_equal(normalized, ""));
    });
    GROUP_EXPECT_PASS2(normalize_group, "normalize newline", {
        char *result = str_normalize("hello\n world!");
        CTASSERT(str_equal(result, "hello\\n world!"));
    });
    GROUP_EXPECT_PANIC(normalize_group, "normalize null", (void)str_normalize(NULL));

    test_group_t normalizen_group = test_group(&suite, "str_normalizen");
    GROUP_EXPECT_PASS2(normalizen_group, "normalizen equals", {
        char *normalized = str_normalizen("hello world!", 12);
        CTASSERT(str_equal(normalized, "hello world!"));
    });
    GROUP_EXPECT_PASS2(normalizen_group, "normalizen empty", {
        char *normalized = str_normalizen("", 0);
        CTASSERT(str_equal(normalized, ""));
    });
    GROUP_EXPECT_PANIC(normalizen_group, "normalizen null", (void)str_normalizen(NULL, 0));

    for (size_t i = 0; i < ESCAPES_LEN; i++)
    {
        pair_t pair = kEscapes[i];

        char *input = str_format(arena, "hello %s world", pair.escaped);
        char *expected = str_format(arena, "hello %s world", pair.unescaped);

        char *result = str_normalize(input);
        char *result_n = str_normalizen(input, strlen(input));

        char *name = str_format(arena, "escaped %s equal (`%s` != `%s`)", pair.escaped, expected, result);
        GROUP_EXPECT_PASS(normalize_group, name, str_equal(result, expected));
        GROUP_EXPECT_PASS(normalizen_group, name, str_equal(result_n, expected));
    }

    ///
    /// erasing
    ///

    test_group_t erase_group = test_group(&suite, "str_erase");
    GROUP_EXPECT_PASS2(erase_group, "erase equals", {
        char test_string[] = "  \t h ello  \n\n world   !   ";
        char *erased = str_erase(test_string, sizeof(test_string), " \t\n\r");
        CTASSERTF(str_equal(erased, "helloworld!"), "got `%s`", erased);
    });

    ///
    /// common prefix
    ///

    vector_t *one_arg = vector_init_arena((char *)"hello", arena);
    const char *one_prefix = str_common_prefix(one_arg);

    vector_t *no_common = vector_of_arena(2, arena);
    vector_set(no_common, 0, (void *)"hello");
    vector_set(no_common, 1, (void *)"world/world2");

    const char *no_common_prefix = str_common_prefix(no_common);

    vector_t *common = vector_of_arena(2, arena);
    vector_set(common, 0, (void *)"hello" NATIVE_PATH_SEPARATOR "stuff");
    vector_set(common, 1, (void *)"hello" NATIVE_PATH_SEPARATOR " world");

    const char *some_prefix = str_common_prefix(common);

    test_group_t group = test_group(&suite, "str_common_prefix");
    GROUP_EXPECT_PASS(group, "one arg", str_equal(one_prefix, "hello"));
    GROUP_EXPECT_PASS(group, "no common", str_equal(no_common_prefix, ""));
    GROUP_EXPECT_PASS(group, "common", str_equal(some_prefix, "hello" NATIVE_PATH_SEPARATOR));

    return test_suite_finish(&suite);
}
