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

    /* if cond goto block */
    OP_COND, 
    
    /* goto block */
    OP_JMP,

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
            operand_t lhs;
            operand_t rhs;
        };

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

/* constant folding optimization pass */
CTU_API bool fold_ir(unit_t *unit);

/* replace blocks of opcodes with more expressive single opcodes */
/* llvm requires this pass to be run */
CTU_API bool raise_ir(unit_t *unit);

/* remove dead steps */
CTU_API bool reduce_ir(unit_t *unit);
