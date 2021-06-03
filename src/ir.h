#pragma once

#include "ast.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    /* for dead code that has been folded away */
    OP_EMPTY,

    /* unary ops */
    OP_ABS,
    OP_NEG,

    /* binary ops */
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_REM,

    /* literals */
    OP_DIGIT,

    /* return a value from a function */
    OP_RETURN,

    OP_CALL
} optype_t;

typedef struct {
    enum { REG, IMM } type;
    union {
        size_t reg;
        int64_t num;
    };
} operand_t;

typedef struct {
    optype_t op;
    bool keep;
    /* associated range */
    size_t range;

    union {
        int64_t num;
        operand_t expr;

        struct {
            operand_t cond;
            size_t label;
        };

        struct {
            operand_t lhs;
            operand_t rhs;
        };

        /* an array of args */
        struct {
            operand_t body;
            operand_t *args;
            size_t total;
        };
    };
} opcode_t;

typedef struct {
    size_t size;
    size_t length;
    opcode_t *ops;
} unit_t;

/* create a TAC from a valid parse tree */
unit_t *ir_gen(node_t *node);

/* constant folding, returns false if nothing was folded */
bool ir_const_fold(unit_t *unit);

/* dead code removal, returns false if nothing was removed */
bool ir_reduce(unit_t *unit);
