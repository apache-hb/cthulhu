#pragma once

#include "ctu/ast/ast.h"
#include "ctu/sema/sema.h"

typedef enum {
    OP_EMPTY, /* optimized out */

    OP_RETURN, /* return from function */
    OP_BINARY, /* binary operation */
    OP_UNARY, /* unary operation */
    OP_CALL, /* calling a function */
    OP_VALUE, /* either a copy or an immediate */
    OP_CONVERT, /* convert a value to another type */

    OP_RESERVE, /* reserve space for type */
    OP_LOAD, /* load from memory */
    OP_STORE, /* store to memory */

    OP_BLOCK, /* start of a basic block */
    OP_BRANCH, /* conditional jump */
    OP_JUMP /* unconditional jump */
} opcode_t;

typedef size_t vreg_t;

typedef struct {
    enum { 
        IMM_BOOL, IMM_INT, IMM_SIZE
    } kind;

    union {
        mpz_t num;
        bool b;
    };
} imm_t;

typedef struct {
    enum { 
        ARG, /* an argument passed to this function */
        VREG, /* a virtual register in the current flow */
        BLOCK, /* an address of a basic block in the current flow */
        
        IMM, /* an immediate value */
        
        FUNC, /* a global function */
        VAR, /* a global variable */
        STRING, /* a string constant stored in the modules string table */
        
        NONE /* nothing */
    } kind;

    union {
        /* virtual register in current flow */
        vreg_t vreg;

        /* argument in current flow */
        vreg_t arg;

        /* basic block in current flow */
        size_t label;

        /* global function index */
        size_t func;

        /* global variable index */
        size_t var;

        /* immediate value */
        imm_t imm;
    };

    /* field offset */
    size_t offset;
} operand_t;

bool operand_is_imm(operand_t op);

bool operand_is_bool(operand_t op);
bool operand_get_bool(operand_t op);
bool operand_is_invalid(operand_t op);

bool operand_as_bool(operand_t op);

void operand_get_int(mpz_t it, operand_t op);

operand_t new_vreg(vreg_t vreg);
operand_t new_block(size_t label);
operand_t new_bool(bool b);
operand_t new_int(mpz_t i);

typedef struct {
    opcode_t opcode;

    type_t *type;

    /* location and source text */
    scanner_t *source;
    where_t where;

    union {
        /* number of elements to reserve */
        size_t size;

        /* OP_RETURN, OP_VALUE, OP_CONVERT */
        struct {
            operand_t value;

            operand_t *args;
            size_t len;
        };

        /* OP_BRANCH */
        struct {
            /**
             * if cond then 
             *  goto block 
             * else 
             *  goto other
             * end
             */
            operand_t cond;

            /* OP_JUMP */
            operand_t block;

            operand_t other;
        };

        /* OP_STORE, OP_LOAD */
        struct {
            operand_t src;
            operand_t dst;
            /* if indexing this is set */
            operand_t offset;
        };

        /* OP_UNARY */
        struct {
            unary_t unary;
            operand_t expr;
        };

        /* OP_BINARY */
        struct {
            binary_t binary;
            operand_t lhs;
            operand_t rhs;
        };
    };
} step_t;

step_t new_jump(operand_t label);
step_t new_value(step_t *last, operand_t value);

type_t *step_type(step_t *step);
bool is_vreg_used(const step_t *step, vreg_t reg);

typedef struct {
    const char *name;
    type_t *type;
} arg_t;

/**
 * a single function
 */
typedef struct {
    /* the name of the function */
    const char *name;

    arg_t *args;
    size_t nargs;

    step_t *steps;
    size_t len, size;

    vreg_t *locals;
    size_t nlocals;

    /* the return type */
    type_t *result;

    /* the parent module this flow is contained in */
    struct module_t *mod;

    bool exported:1;
    bool used:1;
    bool stub:1;
    bool interop:1;
} flow_t;

/**
 * a single variable
 */
typedef struct {
    /* the name of this variable */
    const char *name;

    /* the type of this variable */
    type_t *type;

    bool exported:1;
    bool used:1;
    bool interop:1;
} var_t;

step_t *step_at(flow_t *flow, size_t idx);

/**
 * a compilation unit
 */
typedef struct module_t {
    const char *name;

    /* functions in the current module */
    flow_t *flows;
    size_t nflows;

    /* variables in the current module */
    var_t *vars;
    size_t nvars;

    /* all types declared in this module */
    type_t **types;
    size_t ntypes;

    char **strings;
    size_t nstrings;

    /* required headers and libraries */
    list_t *headers;
    list_t *libs;
} module_t;

size_t num_flows(module_t *mod);
size_t num_vars(module_t *mod);
size_t num_types(module_t *mod);
size_t num_strings(module_t *mod);

module_t *compile_module(const char *name, unit_t unit);
