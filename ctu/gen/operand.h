#pragma once

#include "value.h"

typedef size_t vreg_t;
typedef size_t label_t;

typedef enum {
    IMM, /// an immediate value
    LABEL, /// an address in the current function
    
    VREG, /// a virtual register value in the current block
    ARG, /// a function parameter
    
    ADDRESS, /// an address of a global object
    EMPTY /// an empty or invalid operand
} optype_t;

typedef struct {
    optype_t kind;

    union {
        value_t *imm;
        vreg_t vreg;
        vreg_t arg;
        label_t label;
        struct block_t *block;
    };
} operand_t;

operand_t operand_imm(value_t *imm);
operand_t operand_vreg(vreg_t vreg);
operand_t operand_arg(vreg_t arg);
operand_t operand_label(label_t label);
operand_t operand_address(struct block_t *block);
operand_t operand_empty(void);
