#pragma once

#include "cthulhu/util/map.h"

#define STRING_CASE(name, val) (map_pair_t){ name, (void*)val }
#define MATCH_CASE(name, ...) match_case(name, sizeof(__VA_ARGS__) / sizeof(map_pair_t), __VA_ARGS__)

typedef struct
{
    const char *key;
    void *value;
} map_pair_t;

int match_case(const char *key, size_t numPairs, const map_pair_t *pairs);
