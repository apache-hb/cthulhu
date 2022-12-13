#pragma once

#include "cthulhu/hlir/ops.h"

#include <gmp.h>

typedef struct vector_t vector_t;
typedef struct reports_t reports_t;

typedef struct block_t block_t;
typedef struct flow_t flow_t;
typedef struct step_t step_t;

typedef struct type_t type_t;

typedef enum typekind_t 
{
    eTypeDigit,
    eTypeBool, // either true or false
    eTypeUnit, // only a single value, like void
    eTypeEmpty, // impossible to calculate value, like noreturn

    eTypePointer, // a pointer to another type
    eTypeOpaque, // an opaque pointer to any type
    eTypeString, // a string of characters

    eTypeClosure, // a function pointer

    eTypeStruct, // a possibly unordered list of types in memory

    eTypeTotal
} typekind_t;

typedef struct type_t 
{
    typekind_t kind;
    const char *name;

    union {
        const type_t *ptr;
        struct {
            const type_t *result;
            vector_t *args;
        };
    };
} type_t;

typedef struct imm_t
{
    mpz_t digit;
} imm_t;

typedef enum opkind_t
{
    eOperandEmpty,
    eOperandBlock,
    eOperandReg,
    eOperandLocal,
    eOperandGlobal,
    eOperandFunction,
    eOperandImm,

    eOperandTotal
} opkind_t;

typedef struct operand_t
{
    opkind_t kind;
    const type_t *type;

    union {
        const block_t *bb;
        step_t *reg;
        const flow_t *flow;
        imm_t imm;
        size_t local;
    };
} operand_t;

typedef enum opcode_t
{
    // control flow
    eOpReturn,
    eOpJmp, ///< unconditional jump
    eOpBranch, ///< conditional jump

    // effects
    eOpStore,
    eOpLoad,

    // expressions
    eOpValue,
    eOpUnary,
    eOpBinary,
    eOpCompare,
    eOpCast,
    eOpCall,

    eOpTotal
} opcode_t;

typedef struct step_t
{
    opcode_t opcode;
    const char *id;
    const type_t *type;

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
            operand_t dst;
            operand_t src;
        };

        struct
        {
            union {
                unary_t unary;
                cast_t cast;
            };

            operand_t operand;
        };

        struct
        {
            operand_t *args;
            size_t len;

            operand_t symbol;
        };

        struct
        {
            operand_t cond;
            operand_t label;
            operand_t other;
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
    const type_t *type;
    vector_t *locals; // vector_t<type_t*> 
} flow_t;

typedef struct section_t 
{
    vector_t *globals;
    vector_t *functions;
} section_t;

typedef struct module_t 
{
    section_t symbols;

    section_t imports;
    section_t exports;
} module_t;

module_t *emit_module(reports_t *reports, vector_t *mods);
void eval_module(reports_t *reports, module_t *mod);
