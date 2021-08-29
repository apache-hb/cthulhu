#pragma once

#include "ctu/util/util.h"
#include "ctu/type/type.h"

#include <gmp.h>

typedef struct {
    type_t *type;

    union {
        mpz_t digit;

        bool boolean;
    };
} value_t;

typedef enum {
    
} opkind_t;

typedef struct {
    opkind_t kind;
} operand_t;

typedef enum {
    OP_EMPTY,

    OP_VALUE,
    OP_RETURN
} opcode_t;

typedef struct {
    opcode_t op;
} step_t;

typedef struct {
    const char *name;

} flow_t;

typedef struct {
    const char *name;

    vector_t *flows;
    vector_t *globals;
} module_t;
