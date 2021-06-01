#pragma once

#include "ast.h"

#include <stddef.h>

typedef enum {
    OP_ABS,
    OP_NEG,

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_REM,

    OP_DIGIT,
} optype_t;

typedef struct {
    enum { REG, IMM } type;
    size_t val;
} operand_t;

typedef struct {
    optype_t op;
    
    union {
        operand_t num;
        operand_t expr;
        
        struct {
            operand_t lhs;
            operand_t rhs;
        };
    };
} opcode_t;

typedef struct {
    size_t size;
    size_t length;
    opcode_t *ops;
} unit_t;

unit_t *ir_gen(node_t *node);
