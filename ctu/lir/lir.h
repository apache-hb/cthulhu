#pragma once

#include "ctu/ast/ast.h"
#include "ctu/ast/ops/ops.h"
#include "ctu/util/report.h"
#include "ctu/type/type.h"
#include "attrib.h"

#include <gmp.h>

typedef enum {
    LIR_DIGIT, /// an integer literal
    LIR_STRING, /// a string literal
    LIR_BOOL, /// a boolean literal

    LIR_NAME, /// read from an address
    LIR_ACCESS, /// get the address of a field in an aggregate type
    LIR_BINARY, /// a binary operation
    LIR_UNARY, /// a uary operation
    LIR_CALL, /// calling an address with parameters

    LIR_ASSIGN, /// assign from a value into an address

    LIR_WHILE,
    LIR_BRANCH,
    LIR_STMTS,
    LIR_RETURN,
    LIR_BREAK,

    LIR_VALUE,
    LIR_DEFINE,
    LIR_PARAM, /* a parameter of the current function */

    LIR_MODULE,

    LIR_FORWARD,

    /* error handling */
    LIR_POISON
} leaf_t;

typedef vector_t *symbol_t;

/* lowered intermediate representation */
typedef struct lir_t {
    leaf_t leaf;

    /* the node that this ir originated from */
    node_t *node;

    /* the type this node resolved to */
    const type_t *type;

    union {
        /**
         * a poison error message 
         * error nodes are valid in all places
         * but are considered invalid to the codegen
         * 
         * if emitting a poison error please
         * also place a fatal error into the reporting
         * sink to prevent bad data getting through
         */
        const char *msg;

        /**
         * any integer literal
         */
        mpz_t digit;

        /**
         * an encoded string, null terminated
         */
        const char *str;

        /**
         * a boolean literal
         */
        bool boolean;

        /**
         * a name and an optional offset
         * default the offset to 0, non-0 indicates
         * array access
         * 
         * equivilent to loading from an address
         */
        struct {
            struct lir_t *it;
            size_t offset;
        };

        /**
         * an assignment operation
         * 
         * equivilent to storing to an address
         */
        struct {
            struct lir_t *dst;
            struct lir_t *src;
        };

        /**
         * a binary operation
         * 
         * can be either a mathmatical operation
         * or a logical operation
         */
        struct {
            binary_t binary;
            struct lir_t *lhs;
            struct lir_t *rhs;
        };

        /**
         * a unary operation
         * 
         * can be either a mathmatical operation
         * or a logical operation
         */
        struct {
            unary_t unary;
            struct lir_t *operand;
        };

        /**
         * either a branch or a while loop
         * 
         * a null terminated linked list of branches on other
         * 
         * other can provided on while loops to provide a construct
         * similar to pythons while-else loops
         * 
         * while expr:
         *   stmts
         * else:
         *  stmts # while body was never executed
         */
        struct {
            struct lir_t *cond;
            struct lir_t *then;
            struct lir_t *other;
        };

        /**
         * either a break or a continue
         * if loop is null then it references the current loop
         * otherwise it represents a break or continue with a loop label
         */
        struct lir_t *loop;

        /**
         * a list of statements 
         */
        vector_t *stmts;

        /**
         * function call 
         * 
         * func must be an address
         */
        struct {
            struct lir_t *func;
            vector_t *args;
        };

        /**
         * any named declaration
         * includes variables, functions, arguments, and forward declarations
         */
        struct {
            symbol_t symbol;

            /**
             * if name is null then the symbol is anonymous
             */
            const char *_name;

            /**
             * declaration attributes
             * not needed for parameters
             */
            const attrib_t *attribs;

            union {
                /**
                 * LIR_PARAM
                 * 
                 * the parameter index
                 */
                size_t index;

                /**
                 * LIR_EMPTY
                 * 
                 * a forward declared decl and the type its going to be
                 */
                struct {
                    leaf_t expected;
                    void *ctx;
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
            vector_t *imports; /* imported symbols */
            vector_t *vars; /* defined vars */
            vector_t *funcs; /* defined functions */
        };
    };

    /* internal user data, frontends shouldnt touch this */
    void *data;
} lir_t;

lir_t *lir_forward(node_t *node, const char *name, leaf_t expected, void *ctx);

lir_t *lir_module(node_t *node, 
                  vector_t *imports, 
                  vector_t *vars, 
                  vector_t *funcs);

lir_t *lir_int(node_t *node, const type_t *type, int digit);
lir_t *lir_digit(node_t *node, const type_t *type, mpz_t digit);
lir_t *lir_string(node_t *node, const type_t *type, const char *str);
lir_t *lir_bool(node_t *node, const type_t *type, bool value);

lir_t *lir_access(node_t *node, const type_t *type, lir_t *it, size_t index);
lir_t *lir_name(node_t *node, const type_t *type, lir_t *it);

lir_t *lir_binary(node_t *node, const type_t *type, binary_t binary, lir_t *lhs, lir_t *rhs);
lir_t *lir_unary(node_t *node, const type_t *type, unary_t unary, lir_t *operand);
lir_t *lir_call(node_t *node, 
                const type_t *type, 
                lir_t *func, 
                vector_t *args);

lir_t *lir_assign(node_t *node, lir_t *dst, lir_t *src);
lir_t *lir_while(node_t *node, lir_t *cond, lir_t *then);
lir_t *lir_branch(node_t *node, lir_t *cond, lir_t *then, lir_t *other);
lir_t *lir_stmts(node_t *node, vector_t *stmts);
lir_t *lir_return(node_t *node, lir_t *operand);
lir_t *lir_break(node_t *node, lir_t *loop);

lir_t *lir_poison(node_t *node, const char *msg);

void lir_value(reports_t *reports, 
               lir_t *dst, 
               const type_t *type, 
               lir_t *init);

void lir_define(reports_t *reports, 
                lir_t *dst, 
                const type_t *type, 
                vector_t *locals, 
                lir_t *body);

lir_t *lir_param(node_t *node, const char *name, const type_t *type, size_t index);

void lir_attribs(lir_t *dst, const attrib_t *attribs);

bool lir_ok(const lir_t *lir);
bool lir_is(const lir_t *lir, leaf_t leaf);

vector_t *lir_recurses(lir_t *lir, const lir_t *root);
char *print_lir(const lir_t *lir);
const type_t *lir_type(const lir_t *lir);

bool has_name(const lir_t *lir);
const char *get_name(const lir_t *lir);
