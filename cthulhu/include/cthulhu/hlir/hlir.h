#pragma once

#include <stdbool.h>

#include "cthulhu/hlir/ops.h"
#include "scan/node.h"
#include "std/vector.h"

#include "attribs.h"
#include "digit.h"

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
    eHlirLiteralDigit,  ///< an integer literal
    eHlirLiteralBool,   ///< a boolean literal
    eHlirLiteralString, ///< a string literal

    eHlirName,    ///< a load operation
    eHlirUnary,   ///< a unary operation
    eHlirBinary,  ///< a binary operation
    eHlirCompare, ///< a comparison operation
    eHlirCall,    ///< a function call

    eHlirStmts,  ///< a list of statements
    eHlirBranch, ///< a conditional branch
    eHlirLoop,   ///< a loop on a condition
    eHlirAssign, ///< an assignment
    eHlirReturn, ///< a return statement

    eHlirStruct,  ///< a record type
    eHlirUnion,   ///< an untagged union type
    eHlirDigit,   ///< any integer type
    eHlirBool,    ///< the boolean type
    eHlirString,  ///< any string type
    eHlirVoid,    ///< the void type
    eHlirClosure, ///< the type of a function signature
    eHlirPointer, ///< a pointer to another type
    eHlirArray,   ///< an array of another type
    eHlirType,    ///< the type of all types
    eHlirAlias,   ///< an alias for another type

    eHlirLocal,  ///< a local variable
    eHlirParam,  ///< a function parameter
    eHlirGlobal, ///< a global variable

    eHlirForward,  ///< a forward declaration, should never appear in the final
                   ///< module
    eHlirFunction, ///< a function definition
    eHlirModule,   ///< the toplevel module definition

    eHlirField, ///< a field in a record

    eHlirError, ///< a compilation error

    eHlirTotal
} hlir_kind_t;

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
                      ///< eHlirLiteralDigit
        bool boolean; ///< the value of this boolean literal. active if type ==
                      ///< eHlirLiteralBool
        struct
        {
            const char *string;  ///< the value of this string literal. active if
                                 ///< type == eHlirLiteralString
            size_t stringLength; ///< the length of the string
        };

        struct hlir_t *read; ///< the name of this load operation. active if
                             ///< type == eHlirName

        struct
        {
            struct hlir_t *operand; ///< the operand of this unary operation.
                                    ///< active if type == eHlirUnary
            unary_t unary;          ///< the unary operation to perform
        };

        struct
        {
            struct hlir_t *lhs; ///< the left operand of this operation. active if type ==
                                ///< eHlirBinary || type == eHlirCompare
            struct hlir_t *rhs; ///< the right operand of this operation. active if type
                                ///< == eHlirBinary || type == eHlirCompare

            union {
                binary_t binary;   ///< the binary operation to perform. active if
                                   ///< type == eHlirBinary
                compare_t compare; ///< the comparison operation to perform.
                                   ///< active if type == eHlirCompare
            };
        };

        struct
        {
            struct hlir_t *call; ///< the function to call. active if type == eHlirCall
            vector_t *args;      ///< the arguments to pass to the function.
        };

        vector_t *stmts; ///< the statements in this block. active if type ==
                         ///< eHlirStmts

        struct
        {
            struct hlir_t *dst; ///< the destination of this assignment. active
                                ///< if type == eHlirAssign
            struct hlir_t *src; ///< the source of this assignment.
        };

        /* eHlirBranch|eHlirLoop */
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
 * @brief get the params of a function. only valid to call on eHlirFunction and
 * eHlirClosure
 *
 * @param self the function to get the params of
 * @return the params of the function
 */
vector_t *closure_params(const hlir_t *self);

/**
 * @brief check if a function is variadic. only valid to call on eHlirFunction
 * and eHlirClosure
 *
 * @param self the function to check
 * @return true if the function is variadic
 */
bool closure_variadic(const hlir_t *self);

/**
 * @brief get the return type of a function. only valid to call on eHlirFunction
 * and eHlirClosure
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
