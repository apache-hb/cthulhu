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
    HLIR_ARRAY_INIT, // { expr, expr, ... }

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
    HLIR_FORWARD,
    HLIR_FUNCTION,
    HLIR_VALUE,
    HLIR_MODULE,

    HLIR_FIELD,

    HLIR_ERROR,

    HLIR_TOTAL
} hlir_type_t;

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
    hlir_type_t type; // the type of this node
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

                struct hlir_t *alias;

                /* the aggregate members */
                vector_t *fields;
                
                /* integer type */
                struct {
                    digit_t width;
                    sign_t sign;
                };

                /* closure type */
                struct {
                    vector_t *params;
                    struct hlir_t *result;
                    bool variadic;
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

                /* the type this is expected to be */
                hlir_type_t expected;

                struct {
                    /* the local variables */
                    vector_t *locals;

                    /* the body of this function */
                    struct hlir_t *body;
                };

                /* the initial value */
                struct hlir_t *value;

                struct {
                    vector_t *defines;
                    vector_t *globals;
                    vector_t *types;
                };
            };
        };

        const char *error;
    };
} hlir_t;

///
/// init functions
///

void init_hlir(void);

///
/// querys
///

const hlir_t *typeof_hlir(IN const hlir_t *self);
MAYBE const char *nameof_hlir(IN const hlir_t *self);
bool hlir_is_imported(const hlir_t *self);
bool hlir_is(const hlir_t *self, hlir_type_t type);
bool hlir_can_be(const hlir_t *self, hlir_type_t type);
bool hlir_is_sentinel(const hlir_t *self);

vector_t *closure_params(const hlir_t *self);
bool closure_variadic(const hlir_t *self);
const hlir_t *closure_result(const hlir_t *self);

const char *sign_name(sign_t sign);
const char *digit_name(digit_t digit);

/**
 * @brief create an error
 * 
 * @param node the node that created this error
 * @param error the error message
 * @return hlir_t* the error node
 */
hlir_t *hlir_error(IN const node_t *node, 
                   IN_OPT const char *error);

///
/// type constructors
///

/**
 * @brief create a new integer type
 * 
 * @param node where this type was defined
 * @param name the name of this integer type
 * @param width the width of this integer type
 * @param sign the sign of this integer type
 * @return hlir_t* the constructed integer type
 */
hlir_t *hlir_digit(IN_OPT const node_t *node, 
                   IN_OPT const char *name, 
                   digit_t width, 
                   sign_t sign);

/**
 * @brief construct a new bool type
 * 
 * @param node the node where this type was defined
 * @param name the name of this bool type
 * @return hlir_t* the constructed bool type
 */
hlir_t *hlir_bool(IN_OPT const node_t *node, 
                  IN_OPT const char *name);

/**
 * @brief construct a new string type
 * 
 * @param node the node where this type was defined
 * @param name the name of this string type
 * @return hlir_t* the constructed string type
 */
hlir_t *hlir_string(IN_OPT const node_t *node, 
                    IN_OPT const char *name);

/**
 * @brief construct a new void type
 * 
 * @param node the node where this type was defined
 * @param name the name of this void type
 * @return hlir_t* 
 */
hlir_t *hlir_void(IN_OPT const node_t *node, 
                  IN_OPT const char *name);

/**
 * @brief construct a new closure type
 * 
 * @param node the node where this type was defined
 * @param name the name of this closure type
 * @param params the parameters of this closure type
 * @param result the result of this closure type
 * @param variadic is this a variadic function?
 * @return hlir_t* the constructed closure type
 */
hlir_t *hlir_closure(IN_OPT const node_t *node, 
                     IN_OPT const char *name, 
                     IN vector_t *params, 
                     IN hlir_t *result, 
                     bool variadic);

/**
 * @brief construct a new pointer type
 * 
 * @param node the node where this type was defined
 * @param name the name of this pointer type
 * @param type the type this pointer points to
 * @param indexable can this pointer be indexed?
 * @return hlir_t* the constructed pointer type
 */
hlir_t *hlir_pointer(IN_OPT const node_t *node, 
                     IN_OPT const char *name, 
                     IN hlir_t *type, 
                     bool indexable);

/**
 * @brief construct a new array type
 * 
 * @param node the node where this type was defined
 * @param name the name of this array type
 * @param element the element type of this array
 * @param length the length of this array
 * @return hlir_t* the constructed array type
 */
hlir_t *hlir_array(IN_OPT const node_t *node, 
                   IN_OPT const char *name, 
                   IN hlir_t *element, 
                   IN hlir_t *length);

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

hlir_t *hlir_new_module(IN const node_t *node, 
                        IN const char *name);

hlir_t *hlir_new_function(IN const node_t *node, 
                          IN const char *name,
                          IN vector_t *params,
                          IN hlir_t *result,
                          bool variadic);

hlir_t *hlir_new_value(const node_t *node, const char *name, const hlir_t *type);
hlir_t *hlir_new_struct(const node_t *node, const char *name);
hlir_t *hlir_new_union(const node_t *node, const char *name);
hlir_t *hlir_new_alias(const node_t *node, const char *name);

void hlir_add_local(hlir_t *self, hlir_t *local);
void hlir_add_field(hlir_t *self, hlir_t *field);

void hlir_build_function(hlir_t *self, hlir_t *body);
void hlir_build_value(hlir_t *self, hlir_t *value);

void hlir_build_struct(hlir_t *hlir);
void hlir_build_union(hlir_t *hlir);
void hlir_build_alias(hlir_t *self, hlir_t *type);

hlir_t *hlir_value(const node_t *node, const char *name, const hlir_t *type, hlir_t *value);

void hlir_set_attributes(hlir_t *self, const hlir_attributes_t *attributes);

/**
 * @brief finalize a module and provide it with data
 * 
 * @param self the module to finish
 * @param imports all imported symbols
 * @param values all globals defined in this module
 * @param functions all functions defined in this module
 * @param types all types defined in this module
 */
void hlir_build_module(hlir_t *self, vector_t *values, vector_t *functions, vector_t *types);
