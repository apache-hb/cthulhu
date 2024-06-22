#pragma once

#include "cthulhu/hlir/h2.h"

typedef struct builtin_digit_t {
    const char *name;
    digit_t digit;
    sign_t sign;
} builtin_digit_t;

h2_t *get_builtin_sema(h2_t *root);
