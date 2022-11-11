#include "base/util.h"
#include "std/map.h"
#include "std/str.h"
#include "std/vector.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef size_t ns_t;

static ns_t now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

static void diff(size_t len, size_t prime, ns_t diff)
{
    printf("avg items=%zu,size=%zu: %lu.%06lus\n", len, prime, (unsigned long)(diff / 1000000000),
           (unsigned long)(diff % 1000000000));
}

static void diff2(const char *name, ns_t diff)
{
    printf("%s: %lu.%06lus\n", name, (unsigned long)(diff / 1000000000), (unsigned long)(diff % 1000000000));
}

#define STAGE(name, ...)                                                                                               \
    {                                                                                                                  \
        ns_t start = now();                                                                                            \
        {__VA_ARGS__} ns_t end = now();                                                                                \
        diff2(name, end - start);                                                                                      \
    }

/* a precalculated array of unique names */
static vector_t *idents = NULL;

static const char *kNumbers[] = {"Zero", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine"};

static void vector_flip(vector_t *vec)
{
    size_t start = 0;
    size_t end = vector_len(vec) - 1;
    while (start < end)
    {
        char *lhs = vector_get(vec, start);
        char *rhs = vector_get(vec, end);
        vector_set(vec, end, lhs);
        vector_set(vec, start, rhs);

        start += 1;
        end -= 1;
    }
}

static char *long_name_for(size_t digit)
{
    if (digit == 0)
    {
        return ctu_strdup("Zero");
    }

    vector_t *id = vector_new(8);

    while (digit != 0)
    {
        size_t rem = digit % 10;
        vector_push(&id, (char *)kNumbers[rem]);
        digit = digit / 10;
    }

    vector_flip(id);

    char *out = str_join("_", id);

    vector_delete(id);

    return out;
}

static void init_globals(size_t total)
{
    idents = vector_of(total);
    for (size_t i = 0; i < total; i++)
    {
        char *ident = long_name_for(i);
        vector_set(idents, i, ident);
    }
}

/**
 * https://en.wikipedia.org/wiki/List_of_prime_numbers
 */

static size_t MERSENNE[] = {2,     3,     5,     7,     13,    17,    19,     31,     61,     89,    107,
                            127,   521,   607,   1279,  2203,  2281,  3217,   4253,   4423,   9689,  9941,
                            11213, 19937, 21701, 23209, 44497, 86243, 110503, 132049, 216091, 756839};

static ns_t time_prime(size_t len, size_t prime)
{
    ns_t start = now();

    map_t *map = map_new(prime);
    for (size_t i = 0; i < len; i++)
    {
        char *ident = vector_get(idents, i);
        map_set(map, ident, ident);
    }

    ns_t end = now();

    return end - start;
}

static size_t min_index(ns_t *times, size_t len)
{
    size_t idx = 0;

    ns_t low = SIZE_MAX;
    for (size_t i = 0; i < len; i++)
    {
        ns_t time = times[i];

        if (low > time)
        {
            low = time;
            idx = i;
        }
    }

    return idx;
}

#define AVG_RUNS 10

static ns_t avg_runs(size_t len, size_t size)
{
    ns_t sum = 0;

    printf("averaging: ");
    for (size_t i = 0; i < AVG_RUNS; i++)
    {
        if (i > 0)
        {
            printf(", ");
        }
        printf("%zu/%u", i, AVG_RUNS);
        ns_t run = time_prime(len, size);

        if (run > 2000000000)
        {
            // running unacceptably long, abort
            return 0;
        }

        sum += run;
    }
    printf("\n");

    return sum / AVG_RUNS;
}

#define PRIMES (sizeof(MERSENNE) / sizeof(size_t))

static size_t RUNS[] = {1, 10, 100, 1000, 10000, 100000, 500000, 1000000};

static counters_t kMaxCounters = {.mallocs = SIZE_MAX,
                                  .reallocs = SIZE_MAX,
                                  .frees = SIZE_MAX,

                                  .current = SIZE_MAX,
                                  .peak = SIZE_MAX};

#define TOTAL_RUNS (sizeof(RUNS) / sizeof(size_t))

static counters_t counters[TOTAL_RUNS][PRIMES];
static ns_t times[TOTAL_RUNS][PRIMES];
static size_t winners[TOTAL_RUNS];

static void time_primes(size_t self, size_t len)
{
    size_t i = 0;
    size_t used = 0;

    ns_t last = SIZE_MAX / 2;

    for (; i < PRIMES; i++)
    {
        size_t prime = MERSENNE[i];
        if (prime < (len / 100))
        {
            printf("skipping size=%zu for len=%zu\n", prime, len);
            times[self][used++] = SIZE_MAX;
            counters[self][i] = kMaxCounters;
            continue;
        }

        ns_t avg = avg_runs(len, prime);

        if (avg == 0)
        {
            printf("\naborting size=%zu for len=%zu due to long run\n", prime, len);
            times[self][used++] = SIZE_MAX;
            counters[self][i] = kMaxCounters;
            continue;
        }

        diff(len, prime, avg);

        times[self][used++] = avg;
        counters[self][i] = reset_counters();

        if ((last * 2) < avg)
        {
            printf("passed reasonable end\n");
            break;
        }

        last = avg;
    }

    for (; i < PRIMES; i++)
    {
        counters[self][i] = kMaxCounters;
    }

    size_t idx = min_index(times[self], used);
    size_t prime = MERSENNE[idx];

    winners[self] = prime;
}

static counters_t avg_counters(size_t i)
{
    counters_t out = {0};
    size_t n = 0;

    for (size_t j = 0; j < PRIMES; j++)
    {
        counters_t c = counters[i][j];
        if (c.mallocs == SIZE_MAX)
        {
            continue;
        }

        out.mallocs += c.mallocs;
        out.reallocs += c.reallocs;
        out.frees += c.frees;
        out.current += c.current;
        out.peak += c.peak;

        n += 1;
    }

    out.mallocs /= n;
    out.reallocs /= n;
    out.frees /= n;
    out.current /= n;
    out.peak /= n;

    return out;
}

typedef struct
{
    counters_t most;
    size_t mostIndex;

    counters_t least;
    size_t leastIdx;
} count_pair_t;

static count_pair_t most_allocs(size_t i)
{
    count_pair_t out = {.least = {
                            .mallocs = SIZE_MAX,
                        }};

    for (size_t j = 0; j < PRIMES; j++)
    {
        counters_t c = counters[i][j];
        if (c.mallocs == SIZE_MAX)
        {
            continue;
        }

        if (out.most.mallocs < c.mallocs)
        {
            out.most = c;
            out.mostIndex = j;
        }

        if (out.least.mallocs > c.mallocs)
        {
            out.least = c;
            out.leastIdx = j;
        }
    }

    return out;
}

static count_pair_t most_memory(size_t i)
{
    count_pair_t out = {.least = {
                            .peak = SIZE_MAX,
                        }};

    for (size_t j = 0; j < PRIMES; j++)
    {
        counters_t c = counters[i][j];
        if (c.peak == SIZE_MAX)
        {
            continue;
        }

        if (out.most.peak < c.peak)
        {
            out.most = c;
            out.mostIndex = j;
        }

        if (out.least.peak > c.peak)
        {
            out.least = c;
            out.leastIdx = j;
        }
    }

    return out;
}

int main(void)
{
    STAGE("init", { init_globals(1000000); });

    reset_counters();

    for (size_t i = 0; i < TOTAL_RUNS; i++)
    {
        size_t len = RUNS[i];
        time_primes(i, len);
    }

    for (size_t i = 0; i < TOTAL_RUNS; i++)
    {
        size_t len = RUNS[i];
        size_t prime = winners[i];
        counters_t avg = avg_counters(i);

        count_pair_t malls = most_allocs(i);
        count_pair_t peaks = most_memory(i);

        counters_t peakMallocs = malls.most;
        counters_t leastMallocs = malls.least;

        counters_t peakPeaks = peaks.most;
        counters_t leastPeaks = peaks.least;

        ns_t leastMallocsTime = times[i][malls.leastIdx];
        ns_t leastPeaksTime = times[i][peaks.leastIdx];
        ns_t bestTime = times[i][min_index(times[i], PRIMES)];

        printf("stats for size: %zu\n"
               "performance stats:\n"
               "\tbest prime: %zu\n"
               "\tfastest: %lu.%06lus\n"
               "\ttime for least memory used: %lu.%06lus\n"
               "\ttime for least mallocs: %lu.%06lus\n"
               "memory stats:\n"
               "\tsize that caused the most mallocs: %zu, size that caused the least: %zu\n"
               "\tsize that had the most memory allocated: %zu, size that had the least: %zu\n"
               "\t          | avg             | mallocs (min, max)              | peak (min, max)\n"
               "\tmallocs   : %-15zu | %-15zu %-15zu | %-15zu %-15zu\n"
               "\treallocs  : %-15zu | %-15zu %-15zu | %-15zu %-15zu\n"
               "\tfrees     : %-15zu | %-15zu %-15zu | %-15zu %-15zu\n"
               "\tpeak usage: %-15zu | %-15zu %-15zu | %-15zu %-15zu\n",
               len, prime, (unsigned long)(bestTime / 1000000000), (unsigned long)(bestTime % 1000000000),
               (unsigned long)(leastPeaksTime / 1000000000), (unsigned long)(leastPeaksTime % 1000000000),
               (unsigned long)(leastMallocsTime / 1000000000), (unsigned long)(leastMallocsTime % 1000000000),
               MERSENNE[malls.mostIndex], MERSENNE[malls.leastIdx], MERSENNE[peaks.mostIndex], MERSENNE[peaks.leastIdx],
               avg.mallocs, peakMallocs.mallocs, leastMallocs.mallocs, peakPeaks.mallocs, leastPeaks.mallocs,
               avg.reallocs, peakMallocs.reallocs, leastMallocs.reallocs, peakPeaks.reallocs, leastPeaks.reallocs,
               avg.frees, peakMallocs.frees, leastMallocs.frees, peakPeaks.frees, leastPeaks.frees, avg.peak,
               peakMallocs.peak, leastMallocs.peak, peakPeaks.peak, leastPeaks.peak);
    }
}
