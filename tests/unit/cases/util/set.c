#include "unit/ct-test.h"

#include "setup/memory.h"
#include "arena/arena.h"

#include "std/str.h"
#include "std/set.h"

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
    test_suite_t suite = test_suite_new("set", arena);

    test_group_t set_dedup_group = test_group(&suite, "deduplicates");

    set_t *dedup_set = set_new(3, kTypeInfoString, arena);
    const char *item = set_add(dedup_set, "duplicate");
    for (size_t i = 0; i < 64; i++)
    {
        char *element = arena_strdup("duplicate", arena);
        const char *it = set_add(dedup_set, element);

        /* pointer equality is on purpose */
        GROUP_EXPECT_PASS(set_dedup_group, "items deduplicated", it == item);
    }

    test_group_t set_clash_group = test_group(&suite, "clashes");
    set_t *set = set_new(3, kTypeInfoString, arena);
    for (size_t i = 0; i < SET_ITEMS_COUNT; i++)
    {
        set_add(set, kSetItems[i]);
    }

    for (size_t i = 0; i < SET_ITEMS_COUNT; i++)
    {
        char *name = str_format(arena, "%s in set", kSetItems[i]);
        GROUP_EXPECT_PASS(set_clash_group, name, set_contains(set, kSetItems[i]));
    }

    return test_suite_finish(&suite);
}
