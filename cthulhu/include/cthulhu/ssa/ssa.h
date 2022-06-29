#pragma once

#include "cthulhu/hlir/ops.h"
#include "scan/node.h"

typedef struct hlir_t hlir_t;

typedef enum
{
    OP_EMPTY, // an empty operand
    OP_IMM,   // an immediate value
    OP_REG,   // a register index
    OP_REF    // a symbol reference
} opkind_t;

typedef struct
{
    opkind_t kind;

    union {
        const hlir_t *imm;
        size_t reg;
        struct flow_t *ref;
    };
} op_t;

typedef enum
{
    OP_RETURN,
    OP_CALL,
    OP_BINARY,
    OP_COMPARE,
    OP_JMP,
    OP_COND
} opcode_t;

typedef struct
{
    opcode_t op;
    node_t node;

    union {
        struct
        {
            unary_t unary;
            op_t operand;
        };

        struct
        {
            union {
                binary_t binary;
                compare_t compare;
            };
            op_t lhs;
            op_t rhs;
        };

        struct
        {
            op_t cond;
            op_t then;
            op_t other;
        };

        struct
        {
            op_t call;

            size_t count;
            op_t *args;
        };
    };
} step_t;

typedef struct flow_t
{
    node_t node;

    step_t *steps;
    size_t len;
    size_t total;
} flow_t;
