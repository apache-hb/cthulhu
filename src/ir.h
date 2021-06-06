#pragma once

#include "ast.h"

#include <stddef.h>
#include <stdint.h>

typedef enum {
    /* unary ops */
    OP_ABS, OP_NEG,

    /* either a literal, register, or symbol */
    OP_VALUE,

    /* control flow */
    OP_JMP, OP_PHI, OP_LABEL, OP_CALL, OP_RETURN,

    /* binary math ops */
    OP_ADD, OP_SUB, OP_DIV, OP_MUL, OP_REM
} optype_t;

typedef struct {
    enum { REG, IMM } type;
    union {
        size_t reg;
        int64_t num;
    };
} operand_t;

typedef struct {
    optype_t type;

    union {
        operand_t expr;

        struct {
            operand_t lhs;
            operand_t rhs;
        };

        struct {
            operand_t cond;
            operand_t label;
        };

        struct {
            operand_t body;
            operand_t *args;
            size_t total;
        };
    };
} opcode_t;

typedef struct {
    opcode_t *data;
    size_t size;
    size_t len;
} unit_t;

unit_t ir_emit_node(node_t *node);

opcode_t *ir_opcode_at(unit_t *self, size_t idx);

#if 0
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

    /* either a digit, a register copy, or a symbol */
    OP_VALUE,
    OP_COPY,

    /* control flow */
    OP_LABEL,
    OP_BRANCH,

    /* return a value from a function */
    OP_RETURN,

    OP_CALL
} optype_t;

typedef struct {
    enum { REG, IMM, SYM, LABEL } type;
    union {
        size_t reg;
        int64_t num;
        const char *name;
    };
} operand_t;

typedef struct {
    optype_t op;
    bool keep;
    /* associated range */
    size_t range;

    union {
        operand_t expr;

        struct {
            operand_t cond;
            operand_t label;
        };

        struct {
            operand_t src;
            operand_t dst;
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
    const char *name;
} unit_t;

/* create a TAC from a valid parse tree */
unit_t *ir_gen(node_t *node, const char *name);

/* constant folding, returns false if nothing was folded */
bool ir_const_fold(unit_t *unit);

/* dead code removal, returns false if nothing was removed */
bool ir_reduce(unit_t *unit);

typedef struct {
    unit_t **units;
    size_t len;
    size_t size;
} units_t;

units_t symbol_table(void);
void add_symbol(units_t *units, unit_t *unit);

bool ir_inline(unit_t *unit, units_t *world);
#endif
