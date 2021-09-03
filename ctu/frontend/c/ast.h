#pragma once

#include "ctu/ast/ast.h"

typedef enum {
    C_VALUE
} c_type_t;

typedef struct c_t {
    c_type_t type;
    node_t *node;
} c_t;
