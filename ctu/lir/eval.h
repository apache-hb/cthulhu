#pragma once

#include "lir.h"

typedef struct {
    lir_t *source; /// the toplevel lir that generated this constant

    union {
        mpz_t digit; /// integer literal
        const char *string; /// string literal
        bool boolean; /// boolean literal
        map_t *record; /// struct literal
        vector_t *list; /// array literal
    };
} const_t;
