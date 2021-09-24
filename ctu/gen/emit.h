#pragma once

#include "ctu/util/util.h"
#include "ctu/util/report.h"
#include "ctu/lir/lir.h"

#include "value.h"
#include "operand.h"

typedef enum {
    OP_EMPTY,
    OP_UNARY,
    OP_BINARY,
    OP_CALL,
    OP_RETURN,

    OP_LOAD,
    OP_STORE,

    OP_BRANCH,
    OP_JMP,
    OP_BLOCK
} opcode_t;

typedef struct {
    opcode_t opcode;
    const node_t *node;
    const type_t *type;

    union {
        struct {
            operand_t func;
            operand_t *args;
            size_t len;
        };

        struct {
            operand_t src;
            operand_t dst;
            operand_t offset;
        };

        struct {
            operand_t cond;
            operand_t label;
            operand_t other;
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
    BLOCK_DEFINE,
    BLOCK_SYMBOL,
    BLOCK_STRING
} blocktype_t;

typedef struct block_t {
    blocktype_t kind;

    /* the mangled name of this symbol */
    const char *name;

    /* the node where this block originated */
    const node_t *node;

    /* the type of this block */
    const type_t *type;

    union {
        /* BLOCK_SYMBOL|BLOCK_STRING */
        struct {
            size_t idx; /* index in the string table */
            const char *string; /* the string itself */
        };

        /* BLOCK_DEFINE */
        struct {
            /** 
             * vector_t<struct block_t<BLOCK_SYMBOL>*> 
             * 
             * a vector of all local variables used in the 
             * block.
             */
            vector_t *locals;

            /** 
             * vector_t<struct block_t<BLOCK_SYMBOL>*> 
             * 
             * a vector of all parameters passed into this
             * block.
             */
            vector_t *params;

            /* the computed result of this block if its compile time */
            value_t *value;

            step_t *steps; /// array of steps
            size_t len; /// number of used steps
            size_t size; /// number of allocated steps
        };
    };

    void *data; /// arbitrary data
} block_t;

typedef struct {
    const char *name;

    vector_t *vars;
    vector_t *funcs;

    vector_t *strtab; /* vector_t<block_t<BLOCK_STRING>*> */
    vector_t *imports; /* vector_t<block_t<BLOCK_SYMBOL>*> */
} module_t;

module_t *module_build(reports_t *reports, lir_t *root);
void module_print(FILE *out, module_t *mod);
void module_free(module_t *mod);
