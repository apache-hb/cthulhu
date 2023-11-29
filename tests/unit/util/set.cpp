#include "unit/ct-test.hpp"

#include "base/util.h"
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

static const size_t kSetItemsCount = sizeof(kSetItems) / sizeof(kSetItems[0]);

int main()
{
    test_suite_t::install_panic_handler();

    test_suite_t suite("set");

    test_group_t set_dedup_group = suite.test_group("deduplicates");

    set_t *dedup_set = set_new(3);
    const char *item = set_add(dedup_set, "duplicate");
    for (size_t i = 0; i < 64; i++) {
        char *element = ctu_strdup("duplicate");
        const char *it = set_add(dedup_set, element);

        /* pointer equality is on purpose */
        set_dedup_group.EXPECT_PASS("items deduplicated", it == item);
    }

    test_group_t set_clash_group = suite.test_group("clashes");
    set_t *set = set_new(3);
    for (size_t i = 0; i < kSetItemsCount; i++) {
        set_add(set, kSetItems[i]);
    }

    for (size_t i = 0; i < kSetItemsCount; i++) {
        char *name = format("%s in set", kSetItems[i]);
        set_clash_group.EXPECT_PASS(name, set_contains(set, kSetItems[i]));
    }

    return suite.finish_suite();
}
