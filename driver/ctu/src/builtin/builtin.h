#pragma once

#include "cthulhu/hlir/digit.h"

typedef struct sema_t sema_t;

typedef struct builtin_digit_t {
    const char *name;
    digit_t digit;
    sign_t sign;
} builtin_digit_t;

sema_t *get_builtin_sema(sema_t *root);
