#include "ctu/util/str.h"
#include "ctu/util/util.h"

#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef size_t ns_t;

static ns_t now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

static void diff(const char *name, ns_t diff) {
    printf("%s: %lu.%06lus\n", name, diff / 1000000000, diff % 1000000000);
}

#define STAGE(name, ...) { ns_t start = now(); { __VA_ARGS__ } ns_t end = now(); diff(name, end - start); }

/* a precalculated array of unique names */
vector_t *idents = NULL;

static const char *NAMES[] = {
    "Zero", "One", "Two", "Three",
    "Four", "Five", "Six", "Seven",
    "Eight", "Nine"
};

static void vector_flip(vector_t *vec) {
    size_t start = 0;
    size_t end = vector_len(vec) - 1;
    while (start < end) {
        char *lhs = vector_get(vec, start);
        char *rhs = vector_get(vec, end);
        vector_set(vec, end, lhs);
        vector_set(vec, start, rhs);

        start += 1;
        end -= 1;
    }
}

static char *long_name_for(size_t digit) {
    if (digit == 0) {
        return ctu_strdup("Zero");
    }

    vector_t *id = vector_new(8);

    while (digit != 0) {
        size_t rem = digit % 10;
        vector_push(&id, (char*)NAMES[rem]);
        digit = digit / 10;
    }

    vector_flip(id);

    char *out = strjoin("_", id);

    vector_delete(id);

    return out;
}

static void init_globals(size_t total) {
    idents = vector_of(total);
    for (size_t i = 0; i < total; i++) {
        char *ident = long_name_for(i);
        vector_set(idents, i, ident);
    }
}

/**
 * https://en.wikipedia.org/wiki/List_of_prime_numbers
 */

static size_t MERSENNE[] = { 
    2, 3, 5, 7, 13, 17, 19, 31, 61, 
    89, 107, 127, 521, 607, 1279, 
    2203, 2281, 3217, 4253, 4423, 
    9689, 9941, 11213, 19937, 21701, 
    23209, 44497, 86243, 110503, 
    132049, 216091, 756839
};

static ns_t time_prime(size_t len, size_t prime) {
    ns_t start = now();
    
    map_t *map = map_new(prime);
    for (size_t i = 0; i < len; i++) {
        char *ident = vector_get(idents, i);
        map_set(map, ident, ident);
    }

    ns_t end = now();

    return end - start;
}

static size_t min_index(vector_t *vec) {
    size_t idx = 0;
    
    ns_t low = SIZE_MAX;
    for (size_t i = 0; i < vector_len(vec); i++) {
        ns_t time = (ns_t)vector_get(vec, i);

        if (low > time) {
            low = time;
            idx = i;
        }
    }

    return idx;
}

#define AVG_RUNS 10

static ns_t avg_runs(size_t len, size_t size) {
    ns_t sum = 0;
    
    printf("averaging: ");
    for (size_t i = 0; i < AVG_RUNS; i++) {
        if (i > 0) {
            printf(", ");
        }
        printf("%zu/%u", i, AVG_RUNS);
        ns_t run = time_prime(len, size);

        if (run > 2000000000) {
            // running unacceptably long, abort
            return 0;
        }

        sum += run;
    }
    printf("\n");

    return sum / AVG_RUNS;
}

static void time_primes(size_t len) {
    size_t all = sizeof(MERSENNE) / sizeof(size_t);

    vector_t *times = vector_new(all);

    ns_t last = SIZE_MAX / 2;

    for (size_t i = 0; i < all; i++) {
        size_t prime = MERSENNE[i];
        if (prime < (len / 100)) {
            printf("skipping size=%zu for len=%zu\n", prime, len);
            vector_push(&times, (void *)SIZE_MAX);
            continue;
        }

        ns_t avg = avg_runs(len, prime);
        
        if (avg == 0) {
            printf("\naborting size=%zu for len=%zu due to long run\n", prime, len);
            vector_push(&times, (void *)SIZE_MAX);
            continue;
        }

        char *name = format("avg items=%zu,size=%zu", len, prime);
        diff(name, avg);
        ctu_free(name);
        
        vector_push(&times, (void *)avg);

        if ((last * 2) < avg) {
            printf("passed reasonable end\n");
            break;
        }

        last = avg;
    }

    size_t idx = min_index(times);
    size_t prime = MERSENNE[idx];

    printf("===\n=== winner for items=%zu is size=%zu\n===\n", len, prime);
}

static size_t RUNS[] = {
    1, 10, 100, 1000, 10000, 100000, 500000, 1000000
};

int main(void) {
    STAGE("init", {
        init_globals(1000000);
    });

    size_t total = sizeof(RUNS) / sizeof(size_t);
    for (size_t i = 0; i < total; i++) {
        size_t len = RUNS[i];
        time_primes(len);
    }
}
