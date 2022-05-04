#include "switch.h"

#include <limits.h>
#include <stdint.h>

map_t *map_from_pairs(size_t size, const map_pair_t *pairs)
{
    map_t *map = map_optimal(size);
    
    for (size_t i = 0; i < size; i++)
    {
        map_pair_t pair = pairs[i];
        map_set(map, pair.key, (void*)(intptr_t)pair.value);
    }

    return map;
}

int get_case(map_t *map, const char *key)
{
    return (int)(uintptr_t)map_get_default(map, key, (void*)INT_MAX);
}

int match_case(const char *key, size_t numPairs, const map_pair_t *pairs) 
{
    map_t *map = map_from_pairs(numPairs, pairs);
    int result = get_case(map, key);
    return result;
}
