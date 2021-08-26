#pragma once

#include "ctu/ast/ast.h"

typedef enum {
    LIR_NAME,
    
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

    union {
        /** 
         * LIR_NAME 
         * 
         * points to a variable declaration that this 
         * will read or write to.
         */
        struct lir_t *id;

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
                    vector_t *params;
                    type_t *result;
                    struct lir_t *body;
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
