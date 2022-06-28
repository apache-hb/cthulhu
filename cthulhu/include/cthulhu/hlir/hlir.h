#pragma once

#include <stdbool.h>

#include "cthulhu/hlir/ops.h"
#include "scan/node.h"
#include "std/vector.h"

#include "attribs.h"

#include <gmp.h>

/**
 * @defgroup Hlir HLIR (High Level Intermediate Representation)
 * @brief a high level typed ast format
 * @{
 */

/**
 * the tag for a hlir node
 */
typedef enum
{
    HLIR_DIGIT_LITERAL,  ///< an integer literal
    HLIR_BOOL_LITERAL,   ///< a boolean literal
    HLIR_STRING_LITERAL, ///< a string literal

    HLIR_NAME,    ///< a load operation
    HLIR_UNARY,   ///< a unary operation
    HLIR_BINARY,  ///< a binary operation
    HLIR_COMPARE, ///< a comparison operation
    HLIR_CALL,    ///< a function call

    HLIR_STMTS,  ///< a list of statements
    HLIR_BRANCH, ///< a conditional branch
    HLIR_LOOP,   ///< a loop on a condition
    HLIR_ASSIGN, ///< an assignment
    HLIR_RETURN, ///< a return statement

    HLIR_STRUCT,  ///< a record type
    HLIR_UNION,   ///< an untagged union type
    HLIR_DIGIT,   ///< any integer type
    HLIR_BOOL,    ///< the boolean type
    HLIR_STRING,  ///< any string type
    HLIR_VOID,    ///< the void type
    HLIR_CLOSURE, ///< the type of a function signature
    HLIR_POINTER, ///< a pointer to another type
    HLIR_ARRAY,   ///< an array of another type
    HLIR_TYPE,    ///< the type of all types
    HLIR_ALIAS,   ///< an alias for another type

    HLIR_LOCAL,  ///< a local variable
    HLIR_PARAM,  ///< a function parameter
    HLIR_GLOBAL, ///< a global variable

    HLIR_FORWARD,  ///< a forward declaration, should never appear in the final
                   ///< module
    HLIR_FUNCTION, ///< a function definition
    HLIR_MODULE,   ///< the toplevel module definition

    HLIR_FIELD, ///< a field in a record

    HLIR_ERROR, ///< a compilation error

    HLIR_TOTAL
} hlir_kind_t;

/**
 * @brief the width of an integer type
 */
typedef enum
{
    DIGIT_CHAR,  ///< a range of at least -127 to 127 if signed or 0 to 255 if
                 ///< unsigned
    DIGIT_SHORT, ///< a range of at least -32767 to 32767 if signed or 0 to
                 ///< 65535 if unsigned
    DIGIT_INT,   ///< a range of at least -2147483647 to 2147483647 if signed or 0
                 ///< to 4294967295 if unsigned
    DIGIT_LONG,  ///< a range of at least -9223372036854775807 to
                 ///< 9223372036854775807 if signed or 0 to 18446744073709551615
                 ///< if unsigned

    DIGIT_SIZE, ///< the size of any type
    DIGIT_PTR,  ///< the same width as a pointer
    DIGIT_MAX,  ///< the largest native platform integer

    DIGIT_TOTAL
} digit_t;

/**
 * @brief the sign of an integer type
 */
typedef enum
{
    SIGN_SIGNED,   ///< a signed type
    SIGN_UNSIGNED, ///< an unsigned type

    SIGN_TOTAL
} sign_t;

/**
 * @brief an hlir node
 */
typedef struct hlir_t
{
    hlir_kind_t type;        ///< the type of this node
    node_t location;         ///< the source location that generated this node
    const struct hlir_t *of; ///< the type this hlir evaluates to

    union {
        mpz_t digit;  ///< the value of this integer literal. active if type ==
                      ///< HLIR_DIGIT_LITERAL
        bool boolean; ///< the value of this boolean literal. active if type ==
                      ///< HLIR_BOOL_LITERAL
        struct
        {
            const char *string;  ///< the value of this string literal. active if
                                 ///< type == HLIR_STRING_LITERAL
            size_t stringLength; ///< the length of the string
        };

        struct hlir_t *read; ///< the name of this load operation. active if
                             ///< type == HLIR_NAME

        struct
        {
            struct hlir_t *operand; ///< the operand of this unary operation.
                                    ///< active if type == HLIR_UNARY
            unary_t unary;          ///< the unary operation to perform
        };

        struct
        {
            struct hlir_t *lhs; ///< the left operand of this operation. active if type ==
                                ///< HLIR_BINARY || type == HLIR_COMPARE
            struct hlir_t *rhs; ///< the right operand of this operation. active if type
                                ///< == HLIR_BINARY || type == HLIR_COMPARE

            union {
                binary_t binary;   ///< the binary operation to perform. active if
                                   ///< type == HLIR_BINARY
                compare_t compare; ///< the comparison operation to perform.
                                   ///< active if type == HLIR_COMPARE
            };
        };

        struct
        {
            struct hlir_t *call; ///< the function to call. active if type == HLIR_CALL
            vector_t *args;      ///< the arguments to pass to the function.
        };

        vector_t *stmts; ///< the statements in this block. active if type ==
                         ///< HLIR_STMTS

        struct
        {
            struct hlir_t *dst; ///< the destination of this assignment. active
                                ///< if type == HLIR_ASSIGN
            struct hlir_t *src; ///< the source of this assignment.
        };

        /* HLIR_BRANCH|HLIR_LOOP */
        struct
        {
            struct hlir_t *cond;
            struct hlir_t *then;
            struct hlir_t *other;
        };

        struct
        {
            /* the name of this declaration */
            const char *name;

            /* any attributes this declaration has */
            const hlir_attributes_t *attributes;

            const struct hlir_t *parentDecl; ///< the module that contains this
                                             ///< declaration

            union {
                ///
                /// all types
                ///

                struct
                {
                    const struct hlir_t *alias;
                    bool newtype;
                };

                /* the aggregate members */
                vector_t *fields;

                /* integer type */
                struct
                {
                    digit_t width;
                    sign_t sign;
                };

                /* either a closure type or a function */
                struct
                {
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
                struct
                {
                    struct hlir_t *ptr;
                    bool indexable;
                };

                /* array type */
                struct
                {
                    struct hlir_t *element;
                    struct hlir_t *length;
                };

                ///
                /// all declarations
                ///

                /* the initial value */
                const struct hlir_t *value;

                struct
                {
                    vector_t *functions;
                    vector_t *globals;
                    vector_t *types;
                };
            };
        };
    };
} hlir_t;

/** @} */

///
/// querys
///

/**
 * @defgroup HlirQuerys HLIR querys
 * @ingroup Hlir
 * @brief information queries for HLIR nodes
 * @{
 */

/**
 * @brief check if an hlir node is imported from another module
 *
 * @param self the declaration to check
 * @return true if the declaration is imported
 */
bool hlir_is_imported(const hlir_t *self);

/**
 * @brief get the params of a function. only valid to call on HLIR_FUNCTION and
 * HLIR_CLOSURE
 *
 * @param self the function to get the params of
 * @return the params of the function
 */
vector_t *closure_params(const hlir_t *self);

/**
 * @brief check if a function is variadic. only valid to call on HLIR_FUNCTION
 * and HLIR_CLOSURE
 *
 * @param self the function to check
 * @return true if the function is variadic
 */
bool closure_variadic(const hlir_t *self);

/**
 * @brief get the return type of a function. only valid to call on HLIR_FUNCTION
 * and HLIR_CLOSURE
 *
 * @param self the function to get the return type of
 * @return the return type of the function
 */
const hlir_t *closure_result(const hlir_t *self);

/** @} */

/**
 * @defgroup HlirConstructors HLIR constructors
 * @ingroup Hlir
 * @brief constructors for HLIR nodes
 * @{
 */

/**
 * @brief create an error
 *
 * @param node the node that created this error
 * @param error the error message
 * @return hlir_t* the error node
 */
hlir_t *hlir_error(node_t node, const char *error);

///
/// expression constructors
///

hlir_t *hlir_digit_literal(node_t node, const hlir_t *type, mpz_t value);

hlir_t *hlir_int_literal(node_t node, const hlir_t *type, int value);

hlir_t *hlir_bool_literal(node_t node, const hlir_t *type, bool value);

hlir_t *hlir_string_literal(node_t node, const hlir_t *type, const char *value, size_t length);

hlir_t *hlir_name(node_t node, hlir_t *read);

hlir_t *hlir_unary(node_t node, const hlir_t *type, hlir_t *operand, unary_t unary);
hlir_t *hlir_binary(node_t node, const hlir_t *type, binary_t binary, hlir_t *lhs, hlir_t *rhs);
hlir_t *hlir_compare(node_t node, const hlir_t *type, compare_t compare, hlir_t *lhs, hlir_t *rhs);
hlir_t *hlir_call(node_t node, hlir_t *call, vector_t *args);

hlir_t *hlir_stmts(node_t node, vector_t *stmts);
hlir_t *hlir_branch(node_t node, hlir_t *cond, hlir_t *then, hlir_t *other);
hlir_t *hlir_loop(node_t node, hlir_t *cond, hlir_t *body, hlir_t *other);
hlir_t *hlir_assign(node_t node, hlir_t *dst, hlir_t *src);
hlir_t *hlir_return(node_t node, hlir_t *result);

hlir_t *hlir_field(node_t node, const hlir_t *type, const char *name);

/** @} */
