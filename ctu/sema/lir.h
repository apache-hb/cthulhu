#pragma once

#include "ctu/ast/ast.h"

typedef enum {
    LIR_NAME,
    
    /* integer literal */
    LIR_DIGIT,

    /* forward declaration */
    LIR_EMPTY,

    LIR_VALUE,
    LIR_DEFINE,

    LIR_MODULE
} leaf_t;

/* lowered intermediate representation */
typedef struct lir_t {
    leaf_t leaf;

    /* the node that this ir originated from */
    node_t *node;

    /* the type this node resolved to */
    type_t *type;

    union {
        /** 
         * LIR_NAME 
         * 
         * points to a variable declaration that this 
         * will read or write to.
         */
        struct lir_t *id;

        /**
         * LIR_DIGIT
         * 
         * integer literal
         */
        mpz_t digit;

        struct {
            const char *name;

            union {
                /**
                 * LIR_EMPTY
                 * 
                 * a forward declared decl and the type its going to be
                 */
                leaf_t expected;

                /**
                 * LIR_VALUE
                 * 
                 * a value
                 */
                struct lir_t *init;

                /**
                 * LIR_DEFINE
                 * 
                 * a function
                 */
                struct {
                    vector_t *locals;
                };
            };
        };

        /** 
         * LIR_MODULE 
         * 
         * vector of all global variables and procedures.
         */
        struct {
            vector_t *vars;
            vector_t *funcs;
        };
    };
} lir_t;

lir_t *lir_declare(node_t *node, const char *name, leaf_t expected);
lir_t *lir_module(node_t *node, vector_t *vars, vector_t *funcs);

lir_t *lir_digit(node_t *node, mpz_t digit);

void lir_value(lir_t *dst, type_t *type, lir_t *init);

void lir_resolve(lir_t *dst, type_t *type);
type_t *lir_resolved(lir_t *lir);
