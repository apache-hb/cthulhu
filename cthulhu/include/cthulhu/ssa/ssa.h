#pragma once

#include "cthulhu/hlir/attribs.h"
#include "cthulhu/hlir/string-view.h"
#include "cthulhu/hlir/digit.h"
#include "cthulhu/hlir/ops.h"

#include <stdbool.h>

#include <gmp.h>
#include <mpq.h>

typedef struct vector_t vector_t;
typedef struct reports_t reports_t;

typedef struct ssa_block_t ssa_block_t;
typedef struct ssa_flow_t ssa_flow_t;
typedef struct ssa_step_t ssa_step_t;
typedef struct ssa_operand_t ssa_operand_t;

typedef struct ssa_type_t ssa_type_t;
typedef struct ssa_value_t ssa_value_t;

typedef enum ssa_kind_t
{
#define SSA_TYPE(id, name) id,
#include "cthulhu/ssa/ssa.inc"
    eTypeTotal
} ssa_kind_t;

typedef struct ssa_param_t
{
    const ssa_type_t *type;
    const char *name;
} ssa_param_t;

typedef struct ssa_type_t
{
    ssa_kind_t kind;
    const char *name;

    union {
        // eTypeDigit
        struct {
            digit_t digit;
            sign_t sign;
        };

        // eTypeBool
        /* empty */

        // eTypeString
        /* empty */

        // eTypeUnit
        /* empty */

        // eTypeEmpty
        /* empty */

        // eTypePointer
        const ssa_type_t *ptr;

        // eTypeSignature
        struct {
            const ssa_type_t *result;
            vector_t *args; // vector_t<ssa_param_t*>
            bool variadic;
        };

        // eTypeStruct
        vector_t *fields;
    };
} ssa_type_t;

typedef struct ssa_value_t
{
    const ssa_type_t *type;
    bool initialized;

    union {
        // eTypeDigit
        mpz_t digit;

        // eTypeDecimal
        mpq_t decimal;

        // eTypeBool
        bool boolean;

        // eTypeString
        string_view_t string; // TODO: wrong

        // eTypeUnit
        /* empty */

        // eTypeEmpty
        /* empty */

        // eTypePointer
        /* TODO: implement */

        // eTypeSignature
        /* TODO: implement */

        // eTypeStruct
        /* TODO: implement */
    };
} ssa_value_t;

typedef enum ssa_operand_kind_t
{
#define SSA_OPERAND(id, name) id,
#include "cthulhu/ssa/ssa.inc"
    eOperandTotal
} ssa_operand_kind_t;

typedef struct ssa_operand_t
{
    ssa_operand_kind_t kind;

    union {
        // eOperandEmpty
        /* empty */

        // eOperandBlock
        const ssa_block_t *bb;

        // eOperandReg
        const ssa_step_t *vreg;

        // eOperandLocal
        size_t local;

        // eOperandParam
        size_t param;

        // eOperandGlobal
        const ssa_flow_t *global;

        // eOperandFunction
        const ssa_flow_t *function;

        // eOperandImm
        ssa_value_t *value;

        // eOperandOffset
        struct {
            bool indirect;
            size_t index;
        };
    };

    ssa_operand_t *offset;
} ssa_operand_t;

typedef enum ssa_opcode_t
{
#define SSA_OPCODE(id, name) id,
#include "cthulhu/ssa/ssa.inc"
    eOpTotal
} ssa_opcode_t;

typedef struct ssa_return_t 
{
    ssa_operand_t value;
} ssa_return_t;

typedef struct ssa_jmp_t 
{
    ssa_operand_t label;
} ssa_jmp_t;

typedef struct ssa_branch_t 
{
    ssa_operand_t cond;
    ssa_operand_t truthy;
    ssa_operand_t falsey;
} ssa_branch_t;

typedef struct ssa_store_t
{
    ssa_operand_t dst;
    ssa_operand_t src;
} ssa_store_t;

typedef struct ssa_load_t
{
    ssa_operand_t src;
} ssa_load_t;

typedef struct ssa_addr_t
{
    ssa_operand_t expr;
} ssa_addr_t;

typedef struct ssa_imm_t
{
    ssa_operand_t value;
} ssa_imm_t;

typedef struct ssa_unary_t
{
    unary_t op;
    ssa_operand_t operand;
} ssa_unary_t;

typedef struct ssa_binary_t
{
    binary_t op;
    ssa_operand_t lhs;
    ssa_operand_t rhs;
} ssa_binary_t;

typedef struct ssa_compare_t
{
    compare_t op;
    ssa_operand_t lhs;
    ssa_operand_t rhs;
} ssa_compare_t;

typedef struct ssa_cast_t
{
    cast_t op;
    ssa_operand_t operand;

    const ssa_type_t *type;
} ssa_cast_t;

typedef struct ssa_call_t
{
    ssa_operand_t *args;
    size_t len;

    ssa_operand_t symbol;
} ssa_call_t;

typedef struct ssa_sizeof_t
{
    const ssa_type_t *type;
} ssa_sizeof_t;

typedef struct ssa_step_t
{
    ssa_opcode_t opcode;
    const char *id;
    const ssa_type_t *type;

    union {
        ssa_return_t ret;
        ssa_jmp_t jmp;
        ssa_branch_t branch;

        ssa_store_t store;
        ssa_load_t load;
        ssa_addr_t addr;

        ssa_imm_t imm;
        ssa_unary_t unary;
        ssa_binary_t binary;
        ssa_compare_t compare;
        ssa_cast_t cast;
        ssa_call_t call;

        ssa_sizeof_t size;
    };
} ssa_step_t;

typedef struct ssa_block_t
{
    const char *id;
    vector_t *steps;
} ssa_block_t;

typedef struct ssa_flow_t
{
    const char *name;
    
    linkage_t linkage;
    visibility_t visibility;

    const ssa_type_t *type;

    union {
        // local or exported symbol
        struct
        {
            ssa_block_t *entry;

            union {
                // function
                vector_t *locals;
                
                // global
                const ssa_value_t *value; 
            };
        };

        // imported symbol
        struct
        {
            const char *library;
            const char *symbol;
        };
    };
} ssa_flow_t;

typedef struct section_t
{
    vector_t *types;
    vector_t *globals;
    vector_t *functions;
} section_t;

typedef struct ssa_module_t
{
    section_t symbols;
} ssa_module_t;

ssa_module_t *ssa_gen_module(reports_t *reports, vector_t *mods);

void ssa_emit_module(reports_t *reports, ssa_module_t *mod);

void ssa_opt_module(reports_t *reports, ssa_module_t *mod);
