#include "std/map.h"
#include "std/str.h"
#include "std/vector.h"

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

TEST(test_map_construction, {
    map_t *map = map_new(64);
    SHOULD_PASS("map is not null", map != NULL);
})

TEST(test_map_insertion, {
    map_t *map = map_new(64);

    for (size_t i = 0; i < TOTAL_ITEMS; i++) {
        map_set(map, kSetItems[i], (char*)kSetItems[i]);
    }

    for (size_t i = 0; i < TOTAL_ITEMS; i++) {
        SHOULD_PASS("map has item", map_get(map, kSetItems[i]) != NULL);
    }
})

TEST(test_map_default, {
    map_t *map = map_new(64);
    char world[] = "world";

    /* pointer equality is on purpose */
    SHOULD_PASS("gets default value", map_get_default(map, "hello", world) == world);
})

TEST(test_map_insertion_ptr, {
    map_t *map = map_new(64);

    for (size_t i = 0; i < TOTAL_ITEMS; i++) {
        map_set_ptr(map, kSetItems[i], (char*)kSetItems[i]);
    }

    for (size_t i = 0; i < TOTAL_ITEMS; i++) {
        SHOULD_PASS("map has item", map_get_ptr(map, kSetItems[i]) != NULL);
    }
})

TEST(test_map_iter, {
    map_t *map = map_new(64);

    for (size_t i = 0; i < TOTAL_ITEMS; i++) 
    {
        map_set_ptr(map, kSetItems[i], (char*)kSetItems[i]);
    }

    size_t items = 0;

    map_iter_t iter = map_iter(map);

    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        SHOULD_PASS("map has item", entry.key != NULL);
        SHOULD_PASS("map has item", entry.value != NULL);
        items++;
    }

    SHOULD_PASS("map has items", items == TOTAL_ITEMS);
})

TEST(test_map_entries, {
    map_t *map = map_new(64);
    for (size_t i = 0; i < TOTAL_ITEMS; i++) 
    {
        map_set(map, kSetItems[i], (char*)kSetItems[i]);
    }

    vector_t *entries = map_entries(map);

    SHOULD_PASS("correct entry count", vector_len(entries) == TOTAL_ITEMS);

    for (size_t i = 0; i < vector_len(entries); i++)
    {
        map_entry_t *entry = vector_get(entries, i);
        SHOULD_PASS("map entry correct", str_equal(entry->key, entry->value));
    }
})

TEST(test_map_delete, {
    map_t *map = map_new(64);
    for (size_t i = 0; i < TOTAL_ITEMS; i++) 
    {
        map_set(map, kSetItems[i], (char*)kSetItems[i]);
    }

    for (size_t i = 0; i < TOTAL_ITEMS; i++) 
    {
        map_delete(map, kSetItems[i]);
    }

    vector_t *entries = map_entries(map);

    printf("%zu\n", vector_len(entries));

    SHOULD_PASS("correct entry count", vector_len(entries) == 0);

    for (size_t i = 0; i < TOTAL_ITEMS; i++)
    {
        SHOULD_PASS("map entry correct", map_get(map, kSetItems[i]) == NULL);
    }
})

HARNESS("maps", {
    ENTRY("construction", test_map_construction),
    ENTRY("insertion", test_map_insertion),
    ENTRY("defaults", test_map_default),
    ENTRY("insertion-ptr", test_map_insertion_ptr),
    ENTRY("iter", test_map_iter),
    ENTRY("entries", test_map_entries),
    ENTRY("delete", test_map_delete)
})
