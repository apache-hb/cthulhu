#pragma once

#include <stdbool.h>

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/ops.h"
#include "cthulhu/hlir/arity.h"
#include "cthulhu/hlir/string-view.h"
#include "cthulhu/hlir/attribs.h"
#include "cthulhu/hlir/digit.h"

#include "scan/node.h"
#include "std/vector.h"

#include <gmp.h>

/**
 * @defgroup hlir_t HLIR (High Level Intermediate Representation)
 * @brief a high level typed ast format
 * @{
 */

typedef struct sema_t sema_t;

/**
 * the tag for a hlir node
 */
typedef enum
{
#define HLIR_KIND(ID, _) ID,
#include "hlir-def.inc"

    eHlirTotal
} hlir_kind_t;

/**
 * @brief an hlir node
 */
typedef struct hlir_t
{
    hlir_kind_t type;        ///< the type of this node
    node_t *location;         ///< the source location that generated this node
    const struct hlir_t *of; ///< the type this hlir evaluates to
    void *user;              ///< user data

    union {
        mpz_t digit;  ///< the value of this integer literal. active if type ==
                      ///< eHlirLiteralDigit

        mpq_t decimal;
        
        bool boolean; ///< the value of this boolean literal. active if type ==
                      ///< eHlirLiteralBool

        string_view_t stringLiteral;

        struct hlir_t *read; ///< the name of this load operation. active if
                             ///< type == eHlirLoad

        struct {
            struct hlir_t *operand;

            union {
                unary_t unary;
                builtin_t builtin;
            };
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

        /* eHlirCast */
        struct 
        {
            struct hlir_t *expr;
            cast_t cast;
        };

        /* eHlirBreak | eHlirContinue */
        struct hlir_t *target;

        /* eHlirAccess */
        struct
        {
            struct hlir_t *object;
            struct hlir_t *member;
        };

        /* eHlirIndex */
        struct
        {
            struct hlir_t *array;
            struct hlir_t *index;
        };

        struct
        {
            /* the name of this declaration */
            const char *name;

            /* any attributes this declaration has */
            const hlir_attributes_t *attributes;

            union {
                // unresolved decl
                sema_t *sema;

                ///
                /// all types
                ///

                const struct hlir_t *alias;

                /* the aggregate members */
                /* eHlirEnum|eHlirVariant|eHlirStruct|eHlirUnion */
                /* eHlirVariantCase|eHlirCase */
                struct 
                {
                    /* eHlirVariantCase|eHlirEnum|eHlirStruct|eHlirUnion */
                    vector_t *fields;

                    /* eHlirVariantCase|eHlirCase */
                    const struct hlir_t *value;

                    /* eHlirEnum|eHlirVariant */
                    const struct hlir_t *defaultCase;
                };

                /* integer type */
                struct
                {
                    digit_t width;
                    sign_t sign;
                };

                /* either a closure type or a function */
                struct
                {
                    arity_t arity;
                    vector_t *params;
                    const struct hlir_t *result;

                    /* the local variables */
                    vector_t *locals;

                    /* the body of this function */
                    struct hlir_t *body;
                };

                /* eHlirPointer */
                struct
                {
                    const struct hlir_t *ptr;
                    bool indexable;
                };

                /* eHlirArray */
                struct
                {
                    struct hlir_t *element;
                    struct hlir_t *length;
                };

                ///
                /// all declarations
                ///

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
 * @ingroup hlir_t
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

bool hlir_is_exported(const hlir_t *self);

bool hlir_is_callable(const hlir_t *self);

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
 * @ingroup hlir_t
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
hlir_t *hlir_error(node_t *node, const char *error);

hlir_t *hlir_unresolved(node_t *node, const char *name, sema_t *sema, void *user);

///
/// expression constructors
///

hlir_t *hlir_digit_literal(node_t *node, const hlir_t *type, mpz_t value);

hlir_t *hlir_decimal_literal(node_t *node, const hlir_t *type, mpq_t value);

hlir_t *hlir_int_literal(node_t *node, const hlir_t *type, int value);

hlir_t *hlir_bool_literal(node_t *node, const hlir_t *type, bool value);

hlir_t *hlir_string_literal(node_t *node, const hlir_t *type, struct string_view_t literal);

hlir_t *hlir_name(node_t *node, hlir_t *read);

hlir_t *hlir_unary(node_t *node, unary_t op, hlir_t *operand);
hlir_t *hlir_binary(node_t *node, const hlir_t *type, binary_t binary, hlir_t *lhs, hlir_t *rhs);
hlir_t *hlir_compare(node_t *node, const hlir_t *type, compare_t compare, hlir_t *lhs, hlir_t *rhs);
hlir_t *hlir_call(node_t *node, hlir_t *call, vector_t *args);

hlir_t *hlir_cast(const hlir_t *type, hlir_t *expr, cast_t cast);

hlir_t *hlir_builtin(node_t *node, const hlir_t *type, hlir_t *operand, builtin_t builtin);

hlir_t *hlir_addr(node_t *node, hlir_t *operand);

hlir_t *hlir_stmts(node_t *node, vector_t *stmts);
hlir_t *hlir_branch(node_t *node, hlir_t *cond, hlir_t *then, hlir_t *other);

hlir_t *hlir_loop(node_t *node, hlir_t *cond, hlir_t *body, hlir_t *other);
hlir_t *hlir_break(node_t *node, hlir_t *target);
hlir_t *hlir_continue(node_t *node, hlir_t *target);

hlir_t *hlir_assign(node_t *node, hlir_t *dst, hlir_t *src);
hlir_t *hlir_return(node_t *node, hlir_t *result);

hlir_t *hlir_field(node_t *node, const hlir_t *type, const char *name);
hlir_t *hlir_case(node_t *node, const char *name, hlir_t *value);
hlir_t *hlir_variant_case(node_t *node, const char *name, hlir_t *value, vector_t *fields);

hlir_t *hlir_access(node_t *node, hlir_t *expr, hlir_t *field);
hlir_t *hlir_index(node_t *node, hlir_t *array, hlir_t *index);

/** @} */
