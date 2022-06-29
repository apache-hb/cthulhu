#pragma once

#include "scan/node.h"

typedef enum
{
    VAL_
} value_kind_t;

typedef struct
{
    node_t node;
    value_kind_t kind;
} value_t;
