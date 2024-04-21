#include "unit/ct-test.h"
#include "arena/arena.h"
#include "base/util.h"
#include "setup/memory.h"

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

#define SET_ITEMS_COUNT (sizeof(kSetItems) / sizeof(char*))

int main(void)
{
    test_install_panic_handler();
    test_install_electric_fence();

    arena_t *arena = ctu_default_alloc();
    test_suite_t suite = test_suite_new("map", arena);

    {
        test_group_t group = test_group(&suite, "construction");
        GROUP_EXPECT_PASS(group, "not null", map_new(3, kTypeInfoString, arena) != NULL);
    }

    // insert string
    {
        test_group_t group = test_group(&suite, "insert_str");

        map_t *map = map_new(64, kTypeInfoString, arena);
        for (size_t i = 0; i < SET_ITEMS_COUNT; i++) {
            map_set(map, kSetItems[i], (char*)kSetItems[i]);
        }

        for (size_t i = 0; i < SET_ITEMS_COUNT; i++) {
            char *name = str_format(arena, "%s in map", kSetItems[i]);
            GROUP_EXPECT_PASS(group, name, map_get(map, kSetItems[i]) != NULL);
        }
    }

    // insert ptr
    {
        test_group_t group = test_group(&suite, "insert_ptr");
        map_t *map = map_new(64, kTypeInfoPtr, arena);

        for (size_t i = 0; i < SET_ITEMS_COUNT; i++) {
            map_set(map, kSetItems[i], (char*)kSetItems[i]);
        }

        for (size_t i = 0; i < SET_ITEMS_COUNT; i++) {
            char *name = str_format(arena, "%s in map", kSetItems[i]);
            GROUP_EXPECT_PASS(group, name, map_get(map, kSetItems[i]) != NULL);
        }
    }

    {
        test_group_t group = test_group(&suite, "default value");
        GROUP_EXPECT_PASS2(group, "default value", {
            map_t *map = map_new(64, kTypeInfoString, arena);
            char world[] = "world";

            /* pointer equality is on purpose */
            CTASSERT_ALWAYS(map_get_default(map, "hello", world) == world);
        });
    }

    // iter
    {
        test_group_t group = test_group(&suite, "iter");
        map_t *map = map_new(64, kTypeInfoString, arena);

        for (size_t i = 0; i < SET_ITEMS_COUNT; i++)
        {
            map_set(map, kSetItems[i], (char*)kSetItems[i]);
        }

        size_t items = 0;

        map_iter_t iter = map_iter(map);

        const void *key = NULL;
        void *value = NULL;
        while (CTU_MAP_NEXT(&iter, &key, &value))
        {
            GROUP_EXPECT_PASS(group, "map has item", key != NULL);
            GROUP_EXPECT_PASS(group, "map has item", value != NULL);
            items++;
        }

        GROUP_EXPECT_PASS(group, "map has all items", items == SET_ITEMS_COUNT);
    }

    // get
    {
        test_group_t group = test_group(&suite, "get");
        map_t *map = map_new(64, kTypeInfoString, arena);
        for (size_t i = 0; i < SET_ITEMS_COUNT; i++)
        {
            map_set(map, kSetItems[i], (char*)kSetItems[i]);
        }

        typevec_t *entries = map_entries(map);

        GROUP_EXPECT_PASS(group, "correct entry count", typevec_len(entries) == SET_ITEMS_COUNT);

        for (size_t i = 0; i < typevec_len(entries); i++)
        {
            map_entry_t *entry = (map_entry_t*)typevec_offset(entries, i);
            GROUP_EXPECT_PASS(group, "map entry correct", str_equal((const char*)entry->key, (const char*)entry->value));
        }
    }

    // delete
    {
        test_group_t group = test_group(&suite, "delete");
        map_t *map = map_new(64, kTypeInfoString, arena);
        for (size_t i = 0; i < SET_ITEMS_COUNT; i++)
        {
            map_set(map, kSetItems[i], (char*)kSetItems[i]);
        }

        for (size_t i = 0; i < SET_ITEMS_COUNT; i++)
        {
            map_delete(map, kSetItems[i]);
        }

        typevec_t *entries = map_entries(map);

        GROUP_EXPECT_PASS(group, "correct entry count", typevec_len(entries) == 0);

        for (size_t i = 0; i < SET_ITEMS_COUNT; i++)
        {
            GROUP_EXPECT_PASS(group, "map entry correct", map_get(map, kSetItems[i]) == NULL);
        }
    }

    return test_suite_finish(&suite);
}
