#pragma once

#include "ctu/ast/ast.h"

typedef enum {
    LIR_NAME,
    
    LIR_VALUE,

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
        struct lir_t *name;

        /**
         * LIR_VALUE
         * 
         * a value
         */
        struct {
            bool mut;
            struct lir_t *init;
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

lir_t *lir_module(node_t *node, vector_t *vars, vector_t *funcs);
