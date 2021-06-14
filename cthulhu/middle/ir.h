#pragma once

#include <stddef.h>
#include <stdint.h>

#include "cthulhu/front/ast.h"
#include "cthulhu/util/util.h"

/**
 * our IR looks alot like llvms
 * they mostly got it right so no need to reinvent the wheel.
 */

typedef enum {
    /* %vreg = |op| */
    OP_ABS, 
    
    /* %vreg = op * -1 */
    OP_NEG,

    /* %vreg = op */
    OP_VALUE,

    /* %vreg = call op */
    OP_CALL,

    /* %vreg = select cond true false */
    OP_SELECT,

    /* branch lhs when cond else rhs */
    OP_BRANCH, 
 
    /* jmp block */
    OP_JUMP,

    /* %vreg = [ lhs, rhs ] */
    OP_PHI, 

    /* block: */
    OP_BLOCK,

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
    IMM,
    NAME
} opkind_t;

typedef struct {
    opkind_t kind;
    union {
        /* VREG */
        vreg_t vreg;
        /* IMM */
        int64_t imm;
        /* NAME */
        char *name;
    };
    /* the block this operand is live in */
    size_t block;
} operand_t;

typedef struct {
    size_t block;
    operand_t val;
} branch_t;

typedef struct {
    optype_t kind;

    union {
        /* OP_ABS, OP_NEG, OP_VALUE, OP_RET */
        operand_t expr;

        /* OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_REM */
        struct {
            /* OP_SELECT, OP_BRANCH */
            operand_t cond;

            /* OP_PHI */
            operand_t lhs; /* true leaf */
            operand_t rhs; /* false leaf */
        };

        /* OP_JUMP */
        size_t label;

#if 0
        /* OP_PHI */
        struct {
            branch_t *branches;
            size_t size;
            size_t len;
        };

        /* OP_COND */
        struct {
            operand_t cond;
            size_t block; /* jmp here when cond */
            size_t other; /* otherwise jmp here */
        };

        /* OP_JMP */
        size_t label;
#endif
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
unit_t transform_ast(const char *name, nodes_t *node);
