#pragma once

#include "cthulhu/hlir/ops.h"
#include "cthulhu/hlir/string-view.h"

#include <stdbool.h>

#include <gmp.h>

// TODO: this is all bunk
// rewrite it all

typedef struct vector_t vector_t;
typedef struct reports_t reports_t;

typedef struct block_t block_t;
typedef struct flow_t flow_t;
typedef struct step_t step_t;

typedef struct type_t type_t;

typedef enum typekind_t 
{
#define SSA_TYPE(id, name) id,
#include "cthulhu/ssa/ssa.inc"
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

typedef enum opkind_t
{
#define SSA_OPERAND(id, name) id,
#include "cthulhu/ssa/ssa.inc"
    eOperandTotal
} opkind_t;

typedef struct operand_t
{
    opkind_t kind;
    const type_t *type;

    union {
        const block_t *bb;
        string_view_t string;
        step_t *reg;
        const flow_t *flow;
        mpz_t mpz;
        bool boolean;
        size_t local;
    };
} operand_t;

typedef enum opcode_t
{
#define SSA_OPCODE(id, name) id,
#include "cthulhu/ssa/ssa.inc"
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
    const type_t *type;

    union {
        // function
        struct {
            block_t *entry;
            vector_t *locals; // vector_t<type_t*> 
        };

        // imported symbol
        struct {
            const char *library;
            const char *symbol;
        };
    };
} flow_t;

typedef struct section_t 
{
    vector_t *globals;
    vector_t *functions;
} section_t;

typedef struct module_t 
{
    section_t symbols;

    //section_t imports;
    //section_t exports;
} module_t;

module_t *gen_module(reports_t *reports, vector_t *mods);
void emit_module(reports_t *reports, module_t *mod);

void opt_module(reports_t *reports, module_t *mod);
