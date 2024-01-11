#include "std/map.h"

#include "memory/arena.h"

#include <stdint.h>

static const size_t kSizeAnchors[] = {
    10, 100, 1000,
    10000, 50000, 100000,
    250000, 500000, 750000, 1000000
};

#define TOTAL_SIZE_ANCHORS (sizeof(kSizeAnchors) / sizeof(size_t))

/// @brief select a magic constant from our tuning data
static size_t bucket_for_size(size_t size)
{
    switch (size)
    {
    case 10:
        return 5;
    case 100:
        return 31;
    case 1000:
        return 2203;
    case 10000:
        return 21701;

    case 50000:
    case 100000:
    case 250000:
    case 500000:
        return 216091;

    case 750000:
    case 1000000:
        return 756839;

    default:
        return 2203;
    }
}

static size_t signed_abs(size_t lhs, size_t rhs)
{
    if (rhs > lhs)
    {
        return rhs - lhs;
    }
    return lhs - rhs;
}

static size_t nearest_const(size_t size)
{
    size_t num = 0;
    size_t distance = SIZE_MAX;

    for (size_t i = 0; i < TOTAL_SIZE_ANCHORS; i++)
    {
        size_t d = signed_abs(size, kSizeAnchors[i]);
        if (d < distance)
        {
            distance = d;
            num = kSizeAnchors[i];
        }
        else
        {
            break;
        }
    }

    return num;
}

static size_t select_best_size(size_t size)
{
    size_t near = nearest_const(size);
    size_t bucket = bucket_for_size(near);
    return bucket;
}

USE_DECL
map_t *map_optimal(size_t size, type_info_t info, arena_t *arena)
{
    size_t bucket = select_best_size(size);
    return map_new(bucket, info, arena);
}
