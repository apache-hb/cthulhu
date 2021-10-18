#pragma once

#include "ctu/util/report.h"

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

/**
 * an operand of an instruction
 */
typedef struct {
    optype_t kind;

    union {
        value_t *imm; /// an immediate value, `1`, `true`, `0.1`, etc
        vreg_t vreg; /// a virtual register address. `%N`
        vreg_t arg; /// a function argument
        label_t label; /// a label in the current function `.N:`
        struct block_t *block; /// another block or an imported symbol
    };
} operand_t;

typedef struct {
    operand_t *ops;
    size_t used;
    size_t size;
} oplist_t;

oplist_t *oplist_new(size_t size);
oplist_t *oplist_of(size_t size);
void oplist_push(oplist_t *list, operand_t op);
void oplist_set(oplist_t *list, size_t index, operand_t op);
operand_t oplist_get(oplist_t *list, size_t index);
size_t oplist_len(oplist_t *list);

/**
 * create an immediate value
 * 
 * @param value the value
 * @return the operand
 */
operand_t operand_imm(reports_t *reports, value_t *imm);

/**
 * create a virtual register operand
 * 
 * @param vreg the virtual register
 * @return the operand
 */
operand_t operand_vreg(vreg_t vreg);

/**
 * create an argument operand
 * 
 * @param arg the argument index
 * @return the operand
 */
operand_t operand_arg(vreg_t arg);

/**
 * create a label 
 * 
 * @param label the label
 * @return the operand
 */
operand_t operand_label(label_t label);

/**
 * create an address of another block
 * 
 * @param block the block
 * @return the operand
 */
operand_t operand_address(struct block_t *block);

/**
 * create an operand to fill in later
 * 
 * @return the empty operand
 */
operand_t operand_empty(void);
