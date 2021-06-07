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

typedef void(*ir_apply_func_t)(operand_t, void*);

void ir_opcode_apply(opcode_t *op, ir_apply_func_t func, void *data);

#define APPLY_GENERIC(op, func, data) ir_opcode_apply(op, (ir_apply_func_t)func, data)

opcode_t *ir_opcode_at(unit_t *self, size_t idx);
