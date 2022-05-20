#pragma once

#include "cthulhu/util/map.h"
#include "cthulhu/util/vector.h"
#include "cthulhu/report/report.h"
#include "cthulhu/hlir/hlir.h"

typedef struct flow_t flow_t;

typedef enum
{
    OPERAND_EMPTY,
    OPERAND_VREG,
    OPERAND_CONST,
    OPERAND_ADDR,

    OPERAND_TOTAL
} operand_kind_t;

typedef struct
{
    node_t node;

    operand_kind_t kind;

    union {
        size_t vreg;
        flow_t *addr;
        const hlir_t *value;
    };
} operand_t;

typedef enum
{
    OP_EMPTY,
    OP_CONST,
    OP_UNARY,
    OP_BINARY,

    OP_TOTAL
} op_t;

typedef struct
{
    const hlir_t *type;
    node_t node;

    op_t op;

    union {
        operand_t value;

        struct
        {
            binary_t binary;
            operand_t lhs;
            operand_t rhs;
        };

        struct
        {
            unary_t unary;
            operand_t operand;
        };
    };
} step_t;

typedef struct
{
    size_t size;
    size_t used;
    step_t *steps;
} step_list_t;

typedef struct flow_t
{
    const char *name;
    const hlir_t *type;
    node_t node;

    step_list_t steps;
} flow_t;

typedef struct
{
    const char *name;
    node_t node;

    map_t *globals;
    map_t *functions;
} ssa_t;

vector_t *ssa_compile(reports_t *reports, vector_t *modules);
