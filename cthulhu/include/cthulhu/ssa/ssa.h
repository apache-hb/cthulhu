#pragma once

#include "cthulhu/hlir/ops.h"

typedef struct vector_t vector_t;

typedef struct bb_t bb_t;
typedef struct global_t global_t;
typedef struct flow_t flow_t;
typedef struct step_t step_t;

typedef enum opkind_t
{
    eOperandBB,
    eOperandReg,
    eOperandGlobal,
    eOperandFunction,
    eOperandImm,

    eOperandTotal
} opkind_t;

typedef struct operand_t
{
    opkind_t kind;

    union {
        bb_t *bb;
        step_t *reg;
        global_t *global;
        flow_t *function;
    };
} operand_t;

typedef enum opcode_t 
{
    eOpReturn,
    eOpValue,
    eOpLoad,
    eOpStore,
    eOpUnary,
    eOpBinary,
    eOpCompare,

    eOpTotal
} opcode_t;

typedef struct step_t
{
    opcode_t opcode;

    union {
        struct {
            union {
                binary_t binary;
                compare_t compare;
            };

            operand_t lhs;
            operand_t rhs;
        };

        struct {
            unary_t unary;
            operand_t operand;
        };

        operand_t value;
    };
} step_t;

typedef struct bb_t
{
    vector_t *steps;
} bb_t;

typedef struct global_t
{
    const char *name;

    vector_t *blocks;

    operand_t value;
} global_t;

typedef struct flow_t
{
    const char *name;

    vector_t *blocks;
} flow_t;
