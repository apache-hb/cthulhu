#pragma once

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/util/map.h"
#include "cthulhu/report/report.h"

typedef enum
{
    SSA_EMPTY,
    SSA_CONST,
    SSA_BINARY,
    SSA_UNARY,
    SSA_COMPARE,
    SSA_LOAD,
    SSA_STORE
} ssa_opcode_t;

typedef enum
{
    OP_EMPTY,
    OP_DIGIT,
    OP_BOOL,
    OP_ADDRESS,
    OP_STEP
} ssa_operand_kind_t;

typedef struct
{
    ssa_operand_kind_t kind;
    union
    {
        mpz_t value;
        bool boolean;
        size_t step;
    };
} ssa_operand_t;

typedef struct ssa_step_t
{
    node_t node; ///< the node that created this step
    const hlir_t *type; ///< the resulting type of this step

    ssa_opcode_t opcode;

    union
    {
        struct 
        {
            ssa_operand_t value;
            unary_t unary;
        };

        struct
        {
            ssa_operand_t lhs;
            ssa_operand_t rhs;
            
            union
            {
                binary_t binary;
                compare_t compare;
            };
        };
    };
} ssa_step_t;

typedef struct ssa_flow_t
{
    struct ssa_module_t *parent; ///< the module this flow is *implemented* in

    node_t node; ///< where this flow is implemented
    const hlir_t *type; ///< the type signature of this flow

    ssa_step_t *steps; ///< the steps in this flow
    size_t usedSteps;  ///< the number of used steps in this flow
    size_t totalSteps; ///< the number of total steps in this flow
} ssa_flow_t;

typedef struct ssa_module_t
{
    const char *moduleName; ///< name of this module
    node_t node;     ///< node that this module is associated with

    map_t *modules; ///< modules that this module depends on
                    ///  these can be recursive

    map_t *functions; ///< the functions in this module
    map_t *globals;   ///< the globals in this module
} ssa_module_t;

typedef struct
{
    size_t defaultSteps; ///< the number of steps to use by default
    bool globalsMustBeConst; ///< reduce all globals down to constants
                             ///  fail if globals cannot be made constant
} ssa_settings_t;

vector_t *ssa_compile_modules(reports_t *reports, vector_t *modules, const ssa_settings_t *settings);
