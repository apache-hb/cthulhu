#pragma once

#include "cthulhu/tree/ops.h"

#include <gmp.h>
#include <stdbool.h>

typedef struct reports_t reports_t;
typedef struct vector_t vector_t;
typedef struct typevec_t typevec_t;
typedef struct map_t map_t;
typedef struct set_t set_t;

typedef struct tree_t tree_t;

typedef struct ssa_module_t ssa_module_t;
typedef struct ssa_symbol_t ssa_symbol_t;

typedef struct ssa_block_t ssa_block_t;
typedef struct ssa_step_t ssa_step_t;
typedef struct ssa_operand_t ssa_operand_t;

typedef struct ssa_type_t ssa_type_t;
typedef struct ssa_value_t ssa_value_t;

typedef enum ssa_kind_t {
#define SSA_KIND(ID, NAME) ID,
#include "cthulhu/ssa/ssa.inc"

    eTypeTotal
} ssa_kind_t;

typedef enum ssa_opkind_t {
#define SSA_OPKIND(ID, NAME) ID,
#include "cthulhu/ssa/ssa.inc"

    eOperandTotal
} ssa_opkind_t;

typedef enum ssa_opcode_t {
#define SSA_OPCODE(ID, NAME) ID,
#include "cthulhu/ssa/ssa.inc"

    eOpTotal
} ssa_opcode_t;

///
/// intermediate types
///

typedef struct ssa_param_t {
    const char *name;
    const ssa_type_t *type;
} ssa_param_t;

typedef struct ssa_local_t {
    const char *name;
    const ssa_type_t *type;
} ssa_local_t;

typedef struct ssa_field_t {
    const char *name;
    const ssa_type_t *type;
} ssa_field_t;

///
/// types
///

typedef struct ssa_type_digit_t {
    sign_t sign;
    digit_t digit;
} ssa_type_digit_t;

typedef struct ssa_type_closure_t {
    const ssa_type_t *result;
    typevec_t *params; // typevec_t<ssa_param_t *>
    bool variadic;
} ssa_type_closure_t;

typedef struct ssa_type_pointer_t {
    const ssa_type_t *pointer;
    size_t length;
} ssa_type_pointer_t;

typedef struct ssa_type_record_t {
    typevec_t *fields; // typevec_t<ssa_field_t>
} ssa_type_record_t;

typedef struct ssa_type_storage_t {
    const ssa_type_t *type;
    size_t size;
} ssa_type_storage_t;

typedef struct ssa_type_t {
    ssa_kind_t kind;
    quals_t quals;
    const char *name;

    union {
        ssa_type_digit_t digit;
        ssa_type_closure_t closure;
        ssa_type_pointer_t pointer;
        ssa_type_record_t record;
        ssa_type_storage_t storage;
    };
} ssa_type_t;

typedef struct ssa_value_t {
    const ssa_type_t *type;
    bool init; ///< whether this value has been initialized

    union {
        mpz_t digitValue;
        bool boolValue;

        /* eTypeArray */
        vector_t *data;
    };
} ssa_value_t;

typedef struct ssa_operand_t {
    ssa_opkind_t kind;

    union {
        /* eOperandBlock */
        const ssa_block_t *bb;

        /* eOperandReg */
        struct {
            const ssa_block_t *vregContext;
            size_t vregIndex;
        };

        /* eOperandLocal */
        size_t local;

        /* eOperandParam */
        size_t param;

        /* eOperandConst */
        size_t constant;

        /* eOperandGlobal */
        const ssa_symbol_t *global;

        /* eOperandFunction */
        const ssa_symbol_t *function;

        /* eOperandImm */
        const ssa_value_t *value;
    };
} ssa_operand_t;

typedef struct ssa_store_t {
    ssa_operand_t dst;
    ssa_operand_t src;
} ssa_store_t;

typedef struct ssa_load_t {
    ssa_operand_t src;
} ssa_load_t;

typedef struct ssa_addr_t {
    ssa_operand_t symbol;
} ssa_addr_t;

typedef struct ssa_unary_t {
    ssa_operand_t operand;
    unary_t unary;
} ssa_unary_t;

typedef struct ssa_binary_t {
    ssa_operand_t lhs;
    ssa_operand_t rhs;
    binary_t binary;
} ssa_binary_t;

typedef struct ssa_compare_t {
    ssa_operand_t lhs;
    ssa_operand_t rhs;
    compare_t compare;
} ssa_compare_t;

typedef struct ssa_cast_t {
    ssa_operand_t operand;
    const ssa_type_t *type;
} ssa_cast_t;

typedef struct ssa_call_t {
    ssa_operand_t function;
    typevec_t *args;
} ssa_call_t;

typedef struct ssa_index_t {
    ssa_operand_t array;
    ssa_operand_t index;
} ssa_index_t;

typedef struct ssa_member_t {
    ssa_operand_t object;
    const ssa_field_t *field;
} ssa_member_t;

typedef struct ssa_return_t {
    ssa_operand_t value;
} ssa_return_t;

typedef struct ssa_branch_t {
    ssa_operand_t cond;
    ssa_operand_t then;
    ssa_operand_t other;
} ssa_branch_t;

typedef struct ssa_jump_t {
    ssa_operand_t target;
} ssa_jump_t;

typedef struct ssa_step_t {
    ssa_opcode_t opcode;

    union {
        ssa_store_t store;
        ssa_load_t load;
        ssa_addr_t addr;

        ssa_unary_t unary;
        ssa_binary_t binary;
        ssa_compare_t compare;

        ssa_cast_t cast;
        ssa_call_t call;

        ssa_index_t index;
        ssa_member_t member;

        ssa_return_t ret;
        ssa_branch_t branch;
        ssa_jump_t jump;
    };
} ssa_step_t;

typedef struct ssa_block_t {
    const char *name;
    typevec_t *steps;
} ssa_block_t;

typedef struct ssa_symbol_t {
    tree_link_t linkage;
    visibility_t visibility;

    const char *linkName; ///< external name

    const char *name; ///< internal name
    const ssa_type_t *type; ///< the type of this symbol, must be a closure for functions
    const ssa_value_t *value; ///< the value of this symbol, must always be set for globals

    typevec_t *locals; ///< typevec_t<ssa_type_t>
    typevec_t *params; ///< typevec_t<ssa_type_t>
    vector_t *consts; ///< set_t<ssa_value_t> all constants used by this symbol

    ssa_block_t *entry; ///< entry block

    vector_t *blocks; ///< vector_t<ssa_block_t *>
} ssa_symbol_t;

typedef struct ssa_module_t {
    const char *name;
    vector_t *path; ///< vector<string> the path to this module

    vector_t *types; ///< vector<ssa_type_t> all types used by this module
    vector_t *globals; ///< vector<ssa_symbol_t> all globals declared/imported/exported by this module
    vector_t *functions; ///< vector<ssa_symbol_t> all functions declared/imported/exported by this module
} ssa_module_t;

typedef struct ssa_result_t {
    vector_t *modules; // vector<ssa_module>
    map_t *deps; // map<ssa_symbol, set<ssa_symbol>>
} ssa_result_t;

ssa_result_t ssa_compile(map_t *mods);

///
/// optimization api
///

/**
 * @brief Optimize a given module.
 *
 * @param reports report sink
 * @param mod module to optimize
 */
void ssa_opt(reports_t *reports, ssa_result_t mod);

///
/// rewriting
///

ssa_type_t *ssa_type_bool(const char *name, quals_t quals);
ssa_type_t *ssa_type_digit(const char *name, quals_t quals, sign_t sign, digit_t digit);
ssa_type_t *ssa_type_pointer(const char *name, quals_t quals, ssa_type_t *pointer, size_t length);

///
/// names
///

const char *ssa_type_name(ssa_kind_t kind);
const char *ssa_opkind_name(ssa_opkind_t kind);
const char *ssa_opcode_name(ssa_opcode_t opcode);
