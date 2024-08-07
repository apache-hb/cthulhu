// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_ssa_api.h>

#include "core/compiler.h"

#include "cthulhu/tree/ops.h"

#include <stdbool.h>
#include <gmp.h>

CT_BEGIN_API

/// @defgroup ssa SSA form IR
/// @brief SSA form IR
/// @ingroup runtime
/// @{

typedef struct logger_t logger_t;
typedef struct vector_t vector_t;
typedef struct typevec_t typevec_t;
typedef struct map_t map_t;
typedef struct set_t set_t;
typedef struct arena_t arena_t;

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
#include "ssa.inc"

    eTypeCount
} ssa_kind_t;

typedef enum ssa_opkind_t {
#define SSA_OPERAND(ID, NAME) ID,
#include "ssa.inc"

    eOperandCount
} ssa_opkind_t;

typedef enum ssa_opcode_t {
#define SSA_OPCODE(ID, NAME) ID,
#include "ssa.inc"

    eOpCount
} ssa_opcode_t;

typedef enum ssa_value_state_t {
#define SSA_VALUE(ID, NAME) ID,
#include "ssa.inc"

    eValueCount
} ssa_value_state_t;

///
/// intermediate types
///

/// @brief ssa underlying storage type
///
/// a storage of (type=int, size=4) would be equivalent to `int x[4]` in C
/// a storage of (type=*int, size=4) would be equivalent to `int *x[4]` in C
/// the underlying storage type should match the type of external accessors with a pointer type
/// e.g. if the storage is `int` the accessor should be `int *`
typedef struct ssa_storage_t {
    /// @brief the internal storage type
    const ssa_type_t *type;

    /// @brief the number of elements in the storage
    size_t size;

    /// @brief the qualifiers of the storage
    tree_quals_t quals;
} ssa_storage_t;

typedef struct ssa_param_t {
    const char *name;
    const ssa_type_t *type;
} ssa_param_t;

typedef struct ssa_local_t {
    ssa_storage_t storage;

    const char *name;
    const ssa_type_t *type;
} ssa_local_t;

typedef struct ssa_const_t {
    ssa_storage_t storage;
    const ssa_value_t *value;
} ssa_const_t;

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

typedef struct ssa_case_t {
    const char *name;
    mpz_t value;
} ssa_case_t;

typedef struct ssa_type_enum_t {
    ssa_type_t *underlying;
    typevec_t *cases; // typevec_t<ssa_case_t>
} ssa_type_enum_t;

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

typedef struct ssa_type_t {
    ssa_kind_t kind;
    tree_quals_t quals;
    const char *name;

    union {
        ssa_type_digit_t digit;
        ssa_type_closure_t closure;
        ssa_type_pointer_t pointer;
        ssa_type_record_t record;
        ssa_type_enum_t sum;
    };
} ssa_type_t;

typedef union ssa_literal_value_t {
    /* eTypeDigit */
    mpz_t digit;

    /* eTypeBool */
    bool boolean;

    /* eTypePointer */
    vector_t *data;

    /* eTypeOpaque */
    mpz_t pointer;
} ssa_literal_value_t;

typedef union ssa_relative_value_t {
    const ssa_symbol_t *symbol;
} ssa_relative_value_t;

typedef struct ssa_value_t {
    const ssa_type_t *type;
    ssa_value_state_t value;
    bool init; ///< whether this value has been initialized

    union {
        /* eValueLiteral */
        ssa_literal_value_t literal;

        /* eValueRelocation */
        ssa_relative_value_t relative;
    };
} ssa_value_t;

typedef struct ssa_operand_t {
    ssa_opkind_t kind;

    union {
        /* eOperandBlock */
        const ssa_block_t *bb;

        /* eOperandReg */
        struct {
            const ssa_block_t *vreg_context;
            size_t vreg_index;
        };

        /* eOperandLocal */
        size_t local;

        /* eOperandParam */
        size_t param;

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

typedef struct ssa_offset_t {
    ssa_operand_t array;
    ssa_operand_t offset;
} ssa_offset_t;

typedef struct ssa_member_t {
    ssa_operand_t object;
    size_t index;
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

typedef struct ssa_sizeof_t {
    const ssa_type_t *type;
} ssa_sizeof_t;

typedef struct ssa_alignof_t {
    const ssa_type_t *type;
} ssa_alignof_t;

typedef struct ssa_offsetof_t {
    const ssa_type_t *type;
    size_t index;
} ssa_offsetof_t;

typedef struct ssa_step_t {
    ssa_opcode_t opcode;

    union {
        const ssa_value_t *value;
        ssa_store_t store;
        ssa_load_t load;
        ssa_addr_t addr;

        ssa_unary_t unary;
        ssa_binary_t binary;
        ssa_compare_t compare;

        ssa_cast_t cast;
        ssa_call_t call;

        ssa_offset_t offset;
        ssa_member_t member;

        ssa_return_t ret;
        ssa_branch_t branch;
        ssa_jump_t jump;

        ssa_sizeof_t size_of;
        ssa_alignof_t align_of;
        ssa_offsetof_t offset_of;
    };
} ssa_step_t;

typedef struct ssa_block_t
{
    const char *name;
    typevec_t *steps;
} ssa_block_t;

typedef struct ssa_symbol_t
{
    tree_linkage_t linkage;
    tree_visibility_t visibility;

    const char *linkage_string; ///< external name

    const char *name; ///< internal name
    const ssa_type_t *type; ///< the public facing type of this symbol
    ssa_storage_t storage; ///< the backing storage for this symbol

    const ssa_value_t *value; ///< the value of this symbol, must always be set for globals

    typevec_t *locals; ///< typevec_t<ssa_type_t>
    typevec_t *params; ///< typevec_t<ssa_type_t>

    ssa_block_t *entry; ///< entry block

    vector_t *blocks; ///< vector_t<ssa_block_t *>
} ssa_symbol_t;

typedef struct ssa_module_t {
    const char *name;

    vector_t *types; ///< vector<ssa_type_t> all types used by this module
    vector_t *globals; ///< vector<ssa_symbol_t> all globals declared/imported/exported by this module
    vector_t *functions; ///< vector<ssa_symbol_t> all functions declared/imported/exported by this module
} ssa_module_t;

typedef struct ssa_result_t {
    vector_t *modules; // vector<ssa_module>
    map_t *deps; // map<ssa_symbol, set<ssa_symbol>>
} ssa_result_t;

/// @brief compile a set of trees into their ssa form
///
/// @param mods the modules to compile
/// @param arena the arena to allocate in
///
/// @return the compiled modules
CT_SSA_API ssa_result_t ssa_compile(IN_NOTNULL vector_t *mods, IN_NOTNULL arena_t *arena);

///
/// optimization api
///

/// @brief Optimize a given module.
///
/// @param reports report sink
/// @param mod module to optimize
/// @param arena arena to allocate in
CT_SSA_API void ssa_opt(IN_NOTNULL logger_t *reports, ssa_result_t mod, IN_NOTNULL arena_t *arena);

///
/// rewriting
///

CT_SSA_API ssa_type_t *ssa_type_bool(const char *name, tree_quals_t quals);
CT_SSA_API ssa_type_t *ssa_type_digit(const char *name, tree_quals_t quals, sign_t sign, digit_t digit);
CT_SSA_API ssa_type_t *ssa_type_pointer(const char *name, tree_quals_t quals, ssa_type_t *pointer, size_t length);

///
/// query
///

CT_SSA_API ssa_literal_value_t ssa_value_get_literal(IN_NOTNULL const ssa_value_t *value);
CT_SSA_API bool ssa_value_get_bool(IN_NOTNULL const ssa_value_t *value);
CT_SSA_API void ssa_value_get_digit(IN_NOTNULL const ssa_value_t *value, OUT_NOTNULL mpz_t result);

///
/// names
///

RET_NOTNULL CT_CONSTFN CT_NODISCARD
CT_SSA_API const char *ssa_type_name(STA_IN_RANGE(0, eTypeCount - 1) ssa_kind_t kind);

RET_NOTNULL CT_CONSTFN CT_NODISCARD
CT_SSA_API const char *ssa_opkind_name(STA_IN_RANGE(0, eOperandCount - 1) ssa_opkind_t kind);

RET_NOTNULL CT_CONSTFN CT_NODISCARD
CT_SSA_API const char *ssa_opcode_name(STA_IN_RANGE(0, eOpCount - 1) ssa_opcode_t opcode);

RET_NOTNULL CT_CONSTFN CT_NODISCARD
CT_SSA_API const char *ssa_value_name(STA_IN_RANGE(0, eValueCount - 1) ssa_value_state_t value);

/// @}

CT_END_API
