#define _POSIX_SOURCE

#include "cthulhu/util/util.h"
#include "cthulhu/util/io.h"
#include "cthulhu/util/macros.h"
#include "cthulhu/util/map.h"
#include "cthulhu/util/report.h"
#include "cthulhu/util/str.h"

#include <gmp.h>
#include <stdlib.h>
#include <string.h>

#if ENABLE_TUNING
#include <malloc.h>

static counters_t counters = {
    .mallocs = 0,
    .reallocs = 0,
    .frees = 0,

    .current = 0,
    .peak = 0,
};

counters_t get_counters(void)
{
    return counters;
}

counters_t reset_counters(void)
{
    counters_t out = counters;

    counters.mallocs = 0;
    counters.reallocs = 0;
    counters.frees = 0;
    counters.current = 0;
    counters.peak = 0;

    return out;
}

static void adjust_memory_stats(ssize_t size)
{
    counters.current += size;
    if (counters.current > counters.peak)
    {
        counters.peak = counters.current;
    }
}

static void *tuning_malloc(size_t size)
{
    counters.mallocs++;
    void *result = malloc(size);

    adjust_memory_stats(malloc_usable_size(result));

    return result;
}

static void *tuning_realloc(void *ptr, size_t size)
{
    counters.reallocs++;

    adjust_memory_stats(-malloc_usable_size(ptr));

    void *result = realloc(ptr, size);

    adjust_memory_stats(malloc_usable_size(result));

    return result;
}

static void tuning_free(void *ptr)
{
    counters.frees++;

    adjust_memory_stats(-malloc_usable_size(ptr));

    free(ptr);
}

#define MALLOC(size) tuning_malloc(size)
#define REALLOC(ptr, size) tuning_realloc(ptr, size)
#define FREE(ptr) tuning_free(ptr)

#else
#define MALLOC(size) malloc(size)
#define REALLOC(ptr, size) realloc(ptr, size)
#define FREE(ptr) free(ptr)
#endif

void *ctu_malloc(size_t size)
{
    void *ptr = MALLOC(size);
    CTASSERTF(ptr != NULL, "ctu-malloc of %zu bytes failed", size);
    return ptr;
}

void *ctu_realloc(void *ptr, size_t size)
{
    CTASSERT(ptr != NULL, "ctu-realloc called with NULL pointer");
    void *data = REALLOC(ptr, size);
    CTASSERT(data != NULL, "ctu-realloc failed");
    return data;
}

void ctu_free(void *ptr)
{
    CTASSERT(ptr != NULL, "ctu-free called with NULL pointer");
    FREE(ptr);
}

void *ctu_memdup(const void *ptr, size_t size)
{
    void *out = ctu_malloc(size);
    memcpy(out, ptr, size);
    return out;
}

static void *ctu_gmp_malloc(size_t size)
{
    return ctu_malloc(size);
}

static void *ctu_gmp_realloc(void *ptr, size_t old, size_t size)
{
    UNUSED(old);
    return ctu_realloc(ptr, size);
}

static void ctu_gmp_free(void *ptr, size_t size)
{
    UNUSED(size);
    ctu_free(ptr);
}

void init_gmp(void)
{
    mp_set_memory_functions(ctu_gmp_malloc, ctu_gmp_realloc, ctu_gmp_free);
}

char *ctu_strdup(const char *str)
{
    size_t len = strlen(str) + 1;
    char *out = ctu_malloc(len);
    memcpy(out, str, len);
    return out;
}

char *ctu_strndup(const char *str, size_t len)
{
    char *out = ctu_malloc(len + 1);
    memcpy(out, str, len);
    out[len] = '\0';
    return out;
}

void ctpanic(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    abort();
}

size_t ptrhash(const void *ptr)
{
    uintptr_t key = (uintptr_t)ptr;
    key = (~key) + (key << 18);
    key ^= key >> 31;
    key *= 21;
    key ^= key >> 11;
    key += key << 6;
    key ^= key >> 22;
    return key & SIZE_MAX;
}
