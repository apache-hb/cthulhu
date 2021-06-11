#pragma once

#include <stddef.h>
#include <stdint.h>

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
    OP_REM
} ir_kind_t;

typedef size_t vreg_t;

typedef struct {
    enum { VREG } kind;
    union {
        /* kind = VREG */
        vreg_t vreg;
    };
} operand_t;

typedef struct {
    ir_kind_t kind;

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
    op_t *ops;
    size_t size;
    size_t len;
} flow_t;

/* a compilation unit */
typedef struct {
    flow_t *funcs;
    size_t size;
    size_t len;
} unit_t;
