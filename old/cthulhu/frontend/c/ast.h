#pragma once

#include "cthulhu/ast/ast.h"

#include <gmp.h>

typedef enum {
    C_VALUE
} c_type_t;

typedef struct c_t {
    c_type_t type;
    node_t *node;
} c_t;
