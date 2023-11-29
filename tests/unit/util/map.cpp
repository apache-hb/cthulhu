#include "std/vector.h"
#include "unit/ct-test.hpp"

#include "std/map.h"
#include "std/str.h"

static const char *const kSetItems[] = {
    "a", "b", "c", "d", "e", "f",
    "g", "h", "i", "j", "k", "l",
    "m", "n", "o", "p", "q", "r",
    "s", "t", "u", "v", "w", "x",
    "y", "z", "0", "1", "2", "3",
    "4", "5", "6", "7", "8", "9"
};

static const size_t kSetItemsCount = sizeof(kSetItems) / sizeof(kSetItems[0]);

int main()
{
    test_suite_t::install_panic_handler();

    test_suite_t suite("map");

    suite.test_group("construction")
        .EXPECT_PASS("not null", map_new(3) != NULL);

    // insert string
    {
        test_group_t group = suite.test_group("insert_str");

        map_t *map = map_new(64);
        for (size_t i = 0; i < kSetItemsCount; i++) {
            map_set(map, kSetItems[i], (char*)kSetItems[i]);
        }

        for (size_t i = 0; i < kSetItemsCount; i++) {
            char *name = format("%s in map", kSetItems[i]);
            group.EXPECT_PASS(name, map_get(map, kSetItems[i]) != NULL);
        }
    }

    // insert ptr
    {
        test_group_t group = suite.test_group("insert_ptr");
        map_t *map = map_new(64);

        for (size_t i = 0; i < kSetItemsCount; i++) {
            map_set_ptr(map, kSetItems[i], (char*)kSetItems[i]);
        }

        for (size_t i = 0; i < kSetItemsCount; i++) {
            char *name = format("%s in map", kSetItems[i]);
            group.EXPECT_PASS(name, map_get_ptr(map, kSetItems[i]) != NULL);
        }
    }

    suite.test_group("default value")
        .will_pass("default value", [] {
            map_t *map = map_new(64);
            char world[] = "world";

            /* pointer equality is on purpose */
            return map_get_default(map, "hello", world) == world;
        });

    // iter
    {
        test_group_t group = suite.test_group("iter");
        map_t *map = map_new(64);

        for (size_t i = 0; i < kSetItemsCount; i++)
        {
            map_set_ptr(map, kSetItems[i], (char*)kSetItems[i]);
        }

        size_t items = 0;

        map_iter_t iter = map_iter(map);

        while (map_has_next(&iter))
        {
            map_entry_t entry = map_next(&iter);

            group.EXPECT_PASS("map has item", entry.key != NULL);
            group.EXPECT_PASS("map has item", entry.value != NULL);
            items++;
        }

        group.EXPECT_PASS("map has all items", items == kSetItemsCount);
    }

    // get
    {
        test_group_t group = suite.test_group("get");
        map_t *map = map_new(64);
        for (size_t i = 0; i < kSetItemsCount; i++)
        {
            map_set(map, kSetItems[i], (char*)kSetItems[i]);
        }

        vector_t *entries = map_entries(map);

        group.EXPECT_PASS("correct entry count", vector_len(entries) == kSetItemsCount);

        for (size_t i = 0; i < vector_len(entries); i++)
        {
            map_entry_t *entry = (map_entry_t*)vector_get(entries, i);
            group.EXPECT_PASS("map entry correct", str_equal((const char*)entry->key, (const char*)entry->value));
        }
    }

    // delete
    {
        test_group_t group = suite.test_group("delete");
        map_t *map = map_new(64);
        for (size_t i = 0; i < kSetItemsCount; i++)
        {
            map_set(map, kSetItems[i], (char*)kSetItems[i]);
        }

        for (size_t i = 0; i < kSetItemsCount; i++)
        {
            map_delete(map, kSetItems[i]);
        }

        vector_t *entries = map_entries(map);

        printf("%zu\n", vector_len(entries));

        group.EXPECT_PASS("correct entry count", vector_len(entries) == 0);

        for (size_t i = 0; i < kSetItemsCount; i++)
        {
            group.EXPECT_PASS("map entry correct", map_get(map, kSetItems[i]) == NULL);
        }
    }

    return suite.finish_suite();
}
