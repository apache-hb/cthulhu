#pragma once

#include <stddef.h>
#include <stdint.h>

#include "cthulhu/front/ast.h"

typedef enum {
    /* %vreg = |op| */
    OP_ABS, 
    
    /* %vreg = op * -1 */
    OP_NEG,

    /* %vreg = op */
    OP_VALUE,

    /* if cond goto label */
    OP_JMP, 
    
    /* %vreg = [ lhs, rhs ] */
    OP_PHI, 
    
    /* label: */
    OP_LABEL, 

    /* ret op */
    OP_RET,

    /* %vreg = lhs + rhs */
    OP_ADD, 
    
    /* %vreg = lhs - rhs */
    OP_SUB, 
    
    /* %vreg = lhs / rhs */
    OP_DIV, 
    
    /* %vreg = lhs * rhs */
    OP_MUL, 
    
    /* %vreg = lhs % rhs */
    OP_REM,

    /* optimised away */
    OP_EMPTY
} optype_t;

typedef size_t vreg_t;

typedef enum {
    VREG,
    IMM
} opkind_t;

typedef struct {
    opkind_t kind;
    union {
        /* kind = VREG */
        vreg_t vreg;
        int64_t imm;
    };
} operand_t;

typedef struct {
    optype_t kind;

    union {
        /* OP_ABS, OP_NEG, OP_VALUE, OP_RET */
        operand_t expr;

        /* OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_REM, OP_PHI */
        struct {
            operand_t lhs;
            operand_t rhs;
        };

        /* OP_JMP */
        struct {
            operand_t cond;
            operand_t label;
        };
    };
} op_t;

/* the control flow of a single function */
typedef struct {
    const char *name;

    op_t *ops;
    size_t size;
    size_t len;
} flow_t;

/* a compilation unit */
typedef struct {
    const char *name;
    
    flow_t *flows;
    size_t size;
    size_t len;
} unit_t;

/**
 * convert a compiled file into a compilation unit
 */
unit_t transform_ast(nodes_t *node);
