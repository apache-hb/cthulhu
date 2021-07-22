#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    bool sign;
    char *inner;
} bigint_t;

bigint_t new_bigint(void);
void bigint_assign(bigint_t *self, const char *val);
bool bigint_is_signed(bigint_t *self);
bool bigint_in_range_signed(bigint_t *self, int64_t min, int64_t max);
bool bigint_in_range_unsigned(bigint_t *self, uint64_t max);
