#pragma once

#include "cthulhu/ast/ops.h"
#include "cthulhu/ast/ast.h"

#include "attribs.h"

#include <gmp.h>

/**
 * if anything in this file is changed update the loader
 */
typedef enum {
    /* expressions */
    HLIR_DIGIT_LITERAL, // an integer literal
    HLIR_BOOL_LITERAL, // a boolean literal
    HLIR_STRING_LITERAL, // a string literal

    HLIR_NAME, // expr[offset]
    HLIR_UNARY, // op expr
    HLIR_BINARY, // lhs op rhs
    HLIR_COMPARE, // lhs op rhs
    HLIR_CALL, // expr(args...)

    /* statements */
    HLIR_STMTS, // a list of statements
    HLIR_BRANCH, // if (cond) { then } else { other }
    HLIR_LOOP, // while (cond) { body } else { other }
    HLIR_ASSIGN, // lhs[offset] = rhs

    /* types */
    HLIR_STRUCT, // a c-like struct
    HLIR_UNION, // a c-like union
    HLIR_DIGIT, // an integer type
    HLIR_BOOL, // a boolean type
    HLIR_STRING, // a string type
    HLIR_VOID, // the void type
    HLIR_CLOSURE, // the type of a function signature
    HLIR_POINTER, // a pointer to another type
    HLIR_ARRAY, // an array of another type
    HLIR_TYPE, // the type of all types
    HLIR_ALIAS, // an alias for another type

    /* declarations */
    HLIR_LOCAL,
    HLIR_PARAM,
    HLIR_GLOBAL,

    HLIR_FORWARD,
    HLIR_FUNCTION,
    HLIR_MODULE,

    HLIR_FIELD,

    HLIR_ERROR,

    HLIR_TOTAL
} hlir_kind_t;

typedef enum {
    DIGIT_CHAR,
    DIGIT_SHORT,
    DIGIT_INT,
    DIGIT_LONG,

    DIGIT_SIZE,
    DIGIT_PTR,

    DIGIT_TOTAL
} digit_t;

typedef enum {
    SIGN_SIGNED,
    SIGN_UNSIGNED,
    SIGN_DEFAULT,

    SIGN_TOTAL
} sign_t;

typedef struct hlir_t {
    hlir_kind_t type; // the type of this node
    const node_t *node; // the node span of this hlir
    const struct hlir_t *of; // the type this hlir evaluates to

    union {
        ///
        /// all expressions
        ///
        
        /* HLIR_DIGIT_LITERAL */
        mpz_t digit;

        /* HLIR_BOOL_LITERAL */
        bool boolean;

        /* HLIR_STRING_LITERAL */
        const char *string;

        /* HLIR_NAME */
        struct hlir_t *read;

        /* HLIR_UNARY */
        struct {
            struct hlir_t *operand;
            unary_t unary;
        };

        /* HLIR_BINARY|HLIR_COMPARE */
        struct {
            struct hlir_t *lhs;
            struct hlir_t *rhs;

            union {
                binary_t binary;
                compare_t compare;
            };
        };

        /* HLIR_CALL */
        struct {
            struct hlir_t *call;
            vector_t *args;
        };

        ///
        /// all statements
        ///

        /* HLIR_STMTS */
        vector_t *stmts;

        /* HLIR_ASSIGN */
        struct {
            struct hlir_t *dst;
            struct hlir_t *src;
        };

        /* HLIR_BRANCH|HLIR_LOOP */
        struct {
            struct hlir_t *cond;
            struct hlir_t *then;
            struct hlir_t *other;
        };

        struct {
            /* the name of this declaration */
            const char *name;

            /* any attributes this declaration has */
            const hlir_attributes_t *attributes;

            union {
                ///
                /// all types
                ///

                const struct hlir_t *alias;

                /* the aggregate members */
                vector_t *fields;
                
                /* integer type */
                struct {
                    digit_t width;
                    sign_t sign;
                };

                /* either a closure type or a function */
                struct {
                    vector_t *params;
                    const struct hlir_t *result;
                    bool variadic;
                    
                    /* the local variables */
                    vector_t *locals;

                    union {
                        /* the body of this function */
                        struct hlir_t *body;

                        /* the type this is expected to be */
                        hlir_kind_t expected;
                    };
                };

                /* pointer type */
                struct {
                    struct hlir_t *ptr;
                    bool indexable;
                };

                /* array type */
                struct {
                    struct hlir_t *element;
                    struct hlir_t *length;
                };

                ///
                /// all declarations
                ///

                /* the initial value */
                const struct hlir_t *value;

                /* the index of this local */
                size_t index;

                struct {
                    vector_t *functions;
                    vector_t *globals;
                    vector_t *types;
                };
            };
        };

        const char *error;
    };
} hlir_t;

///
/// querys
///

bool hlir_is_imported(const hlir_t *self);
bool hlir_can_be(const hlir_t *self, hlir_kind_t type);
bool hlir_is_sentinel(const hlir_t *self);

vector_t *closure_params(const hlir_t *self);
bool closure_variadic(const hlir_t *self);
const hlir_t *closure_result(const hlir_t *self);

/**
 * @brief create an error
 * 
 * @param node the node that created this error
 * @param error the error message
 * @return hlir_t* the error node
 */
hlir_t *hlir_error(IN const node_t *node, IN_OPT const char *error);

///
/// expression constructors
///

hlir_t *hlir_digit_literal(IN const node_t *node, 
                           IN const hlir_t *type, 
                           IN mpz_t value);

hlir_t *hlir_int_literal(IN_OPT const node_t *node,
                         IN_OPT const hlir_t *type,
                         int value);

hlir_t *hlir_bool_literal(IN const node_t *node, 
                          IN const hlir_t *type, 
                          bool value);

hlir_t *hlir_string_literal(IN const node_t *node,
                            IN const hlir_t *type,
                            IN const char *value);

hlir_t *hlir_name(const node_t *node, hlir_t *read);

hlir_t *hlir_unary(const node_t *node, const hlir_t *type, hlir_t *operand, unary_t unary);
hlir_t *hlir_binary(const node_t *node, const hlir_t *type, binary_t binary, hlir_t *lhs, hlir_t *rhs);
hlir_t *hlir_compare(const node_t *node, const hlir_t *type, compare_t compare, hlir_t *lhs, hlir_t *rhs);
hlir_t *hlir_call(const node_t *node, hlir_t *call, vector_t *args);

hlir_t *hlir_stmts(const node_t *node, vector_t *stmts);
hlir_t *hlir_branch(const node_t *node, hlir_t *cond, hlir_t *then, hlir_t *other);
hlir_t *hlir_loop(const node_t *node, hlir_t *cond, hlir_t *body, hlir_t *other);
hlir_t *hlir_assign(const node_t *node, hlir_t *dst, hlir_t *src);

hlir_t *hlir_field(const node_t *node, const hlir_t *type, const char *name);

void hlir_set_attributes(hlir_t *self, const hlir_attributes_t *attributes);
