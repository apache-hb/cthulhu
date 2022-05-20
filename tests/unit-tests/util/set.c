#include "cthulhu/util/set.h"
#include "cthulhu/util/str.h"
#include "ct-test.h"

static const char *kSetItems[] = {
    "a", "b", "c", "d", "e", "f", 
    "g", "h", "i", "j", "k", "l", 
    "m", "n", "o", "p", "q", "r", 
    "s", "t", "u", "v", "w", "x", 
    "y", "z", "0", "1", "2", "3",
    "4", "5", "6", "7", "8", "9"
};

#define TOTAL_ITEMS (sizeof(kSetItems) / sizeof(const char *))

TEST(test_set_clashes, {
    set_t *set = set_new(3, alloc_global());
    for (size_t i = 0; i < TOTAL_ITEMS; i++) {
        (void)set_add(set, kSetItems[i]);
    }

    for (size_t i = 0; i < TOTAL_ITEMS; i++) {
        char *name = format("%s in set", kSetItems[i]);
        SHOULD_PASS(name, set_contains(set, kSetItems[i]));
    }
})

TEST(test_set_deduplicates, {
    set_t *set = set_new(3, alloc_global());
    const char *item = set_add(set, "duplicate");
    for (size_t i = 0; i < 64; i++) {
        char *element = ctu_strdup("duplicate");
        const char *it = set_add(set, element);

        /* pointer equality is on purpose */
        SHOULD_PASS("items deduplicated", it == item);
    }
})

HARNESS("sets", {
    ENTRY("clashes", test_set_clashes),
    ENTRY("deduplicates", test_set_deduplicates)
})
