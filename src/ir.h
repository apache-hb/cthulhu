#pragma once

#include <stddef.h>

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_REM,
    OP_DIGIT,

    OP_LABEL,
    OP_COPY,
    OP_STORE,
    OP_LOAD
} optype_t;

typedef struct {
    enum { REG, IMM } type;
    size_t val;
} operand_t;

typedef struct {
    optype_t op;
    
    union {
        operand_t num;

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

unit_t *new_unit(void);
size_t unit_add(unit_t *unit, opcode_t op);