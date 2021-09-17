#pragma once

#include "ctu/util/util.h"
#include "ctu/util/report.h"
#include "ctu/lir/lir.h"

#include "value.h"

typedef enum {
    IMM, /// an immediate value
    VREG, /// a virtual register value in the current block
    LABEL, /// an address in the current function
    LOCAL, /// a function local variable
    ADDRESS, /// an address of a global object
    EMPTY /// an empty or invalid operand
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
    node_t *node;
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

typedef struct block_t {
    const char *name;

    /* the return type of this */
    const type_t *result;
    const value_t *value;

    operand_t *vars;
    size_t locals;

    size_t len;
    size_t size;
    step_t *steps;
} block_t;

typedef struct {
    const char *name;

    vector_t *vars;
    vector_t *funcs;
} module_t;

module_t *module_build(reports_t *reports, lir_t *root);
void module_print(FILE *out, module_t *mod);

typedef struct {
    /* the mangled name of this symbol */
    const char *name;

    /** 
     * vector_t<type_t*> 
     * 
     * a vector of all local variables used in the 
     * block.
     */
    vector_t *locals;

    /** 
     * vector_t<type_t*> 
     * 
     * a vector of all parameters passed into this
     * block.
     */
    vector_t *params;

    /* the return type of this function */
    const type_t *result;

    /* the computed result of this block if its compile time */
    const value_t *value;

    step_t *steps;
    size_t len;
    size_t size;
} block2_t;
