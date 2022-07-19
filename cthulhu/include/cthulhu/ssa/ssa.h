#pragma once

#include "cthulhu/hlir/digit.h"
#include "cthulhu/hlir/ops.h"

#include <stdbool.h>

#include <gmp.h>

typedef struct reports_t reports_t;
typedef struct vector_t vector_t;
typedef struct stream_t stream_t;
typedef struct flow_t flow_t;

typedef size_t vreg_t;

typedef enum
{
    eOperandEmpty, // an empty operand
    eOperandImm,   // an immediate value
    eOperandReg,   // a register index
    eOperandRef,    // a symbol reference
    eOperandLocal, // a local variable
    eOperandArg    // a function argument
} opkind_t;

typedef enum
{
    eLiteralInt,
    eLiteralBool,
    eLiteralVoid,
    eLiteralString,
    eLiteralEmpty
} literal_t;

typedef struct
{
    digit_t width;
    sign_t sign;
} int_t;

typedef struct
{
    literal_t kind;

    union {
        int_t digitKind;
    };
} type_t;

typedef struct
{
    type_t type;

    union {
        mpz_t digit;
        bool boolean;
        const char *string;
    };
} value_t;

typedef struct
{
    opkind_t kind;

    union {
        value_t value;
        vreg_t reg;
        flow_t *ref;
        size_t local;
    };
} operand_t;

typedef enum
{
    eOpReturn,
    eOpValue,
    eOpCall,
    eOpBinary,
    eOpCompare,
    eOpJmp,
    eOpCond,
    eOpLoad,
    eOpStore
} opcode_t;

typedef struct
{
    opcode_t op;
    type_t type;

    union {
        operand_t result;

        struct
        {
            unary_t unary;
            operand_t operand;
        };

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
            operand_t cond;
            operand_t then;
            operand_t other;
        };

        struct
        {
            operand_t call;

            size_t count;
            operand_t *args;
        };

        struct
        {
            operand_t src;
            operand_t dst;
        };
    };
} step_t;

typedef struct flow_t
{
    const char *name;

    size_t stepsSize;
    size_t stepsLen;
    step_t *steps;

    vector_t *params; // all function parameters
    vector_t *locals; // all local variables
} flow_t;

typedef struct module_t
{
    vector_t *imports;   ///< imported symbols
    vector_t *exports;   ///< exported symbols
    vector_t *internals; ///< internal functions

    flow_t *cliEntry;
    flow_t *guiEntry;
} module_t;

module_t *ssa_compile(reports_t *reports, vector_t *modules);

stream_t *ssa_debug(reports_t *reports, module_t *mod);
