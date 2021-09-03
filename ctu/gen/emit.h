#pragma once

#include "ctu/util/util.h"
#include "ctu/type/type.h"
#include "ctu/lir/lir.h"

typedef struct {
    type_t *type;

    union {
        mpz_t digit;
    };
} value_t;

typedef enum {
    IMM,
    VREG,
    LABEL,
    ADDRESS,
    EMPTY
} optype_t;

typedef struct {
    optype_t kind;

    union {
        value_t *imm;
        size_t vreg;
        size_t label;
        struct block_t *block;
    };
} operand_t;

typedef enum {
    OP_EMPTY,
    OP_UNARY,
    OP_BINARY,
    OP_RETURN,

    OP_LOAD,
    OP_STORE
} opcode_t;

typedef struct {
    opcode_t opcode;
    node_t *node;
    type_t *type;

    union {
        struct {
            operand_t src;
            operand_t dst;
            operand_t offset;
        };
        
        struct {
            unary_t unary;
            operand_t operand;
        };

        struct {
            binary_t binary;
            operand_t lhs;
            operand_t rhs;
        };
    };
} step_t;

typedef enum {
    BLOCK_VALUE,
    BLOCK_FLOW
} block_type_t;

typedef struct block_t {
    const char *name;
    block_type_t type;

    /* the return type of this */
    type_t *result;

    size_t len;
    size_t size;
    step_t *steps;
} block_t;

typedef struct {
    const char *name;

    vector_t *blocks;
} module_t;

module_t *module_build(lir_t *root);
void module_print(FILE *out, module_t *mod);
