#pragma once

#include "ctu/ast/ast.h"

typedef enum {
    /* integer literal */
    LIR_DIGIT,

    /* binary expresion */
    LIR_BINARY,
    LIR_UNARY,

    /* forward declaration */
    LIR_EMPTY,

    LIR_VALUE,
    LIR_DEFINE,

    LIR_MODULE,

    LIR_POISON
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
         * LIR_POISON
         * 
         */
        const char *msg;

        /**
         * LIR_DIGIT
         * 
         * integer literal
         */
        mpz_t digit;

        struct {
            binary_t binary;
            struct lir_t *lhs;
            struct lir_t *rhs;
        };

        struct {
            unary_t unary;
            struct lir_t *operand;
        };

        struct {
            const char *name;

            union {
                /**
                 * LIR_EMPTY
                 * 
                 * a forward declared decl and the type its going to be
                 */
                struct {
                    leaf_t expected;
                    struct sema_t *sema;
                };

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

lir_t *lir_declare(node_t *node, const char *name, leaf_t expected, struct sema_t *sema);
lir_t *lir_module(node_t *node, vector_t *vars, vector_t *funcs);

lir_t *lir_int(node_t *node, int digit);
lir_t *lir_digit(node_t *node, mpz_t digit);

lir_t *lir_binary(node_t *node, binary_t binary, lir_t *lhs, lir_t *rhs);
lir_t *lir_unary(node_t *node, unary_t unary, lir_t *operand);

lir_t *lir_poison(node_t *node, const char *msg);

void lir_value(lir_t *dst, type_t *type, lir_t *init);
void lir_begin(lir_t *dst, leaf_t leaf);
bool lir_ok(lir_t *lir);
bool lir_is(lir_t *lir, leaf_t leaf);

vector_t *lir_recurses(lir_t *lir, const lir_t *root);
