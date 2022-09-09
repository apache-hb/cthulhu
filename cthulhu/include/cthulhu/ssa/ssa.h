#pragma once

#include "cthulhu/hlir/ops.h"

#include <gmp.h>

typedef struct vector_t vector_t;
typedef struct reports_t reports_t;

typedef struct block_t block_t;
typedef struct flow_t flow_t;
typedef struct step_t step_t;

typedef struct imm_t
{
    mpz_t digit;
} imm_t;

typedef enum opkind_t
{
    eOperandEmpty,
    eOperandBlock,
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
        const block_t *bb;
        step_t *reg;
        const flow_t *flow;
        imm_t imm;
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
    const char *id;

    union {
        struct
        {
            union {
                binary_t binary;
                compare_t compare;
            };

            operand_t lhs;
            operand_t rhs;
        };

        struct
        {
            unary_t unary;
            operand_t operand;
        };

        operand_t value;
    };
} step_t;

typedef struct block_t
{
    const char *id;
    vector_t *steps;
} block_t;

typedef struct flow_t
{
    const char *name;

    block_t *entry;
} flow_t;

typedef struct module_t 
{
    vector_t *flows;
} module_t;

module_t *emit_module(reports_t *reports, vector_t *mods);
void eval_module(reports_t *reports, module_t *mod);
