#pragma once

#include "cthulhu/hlir2/ops.h"

#include <stdbool.h>
#include <gmp.h>

typedef struct reports_t reports_t;

typedef struct vector_t vector_t;
typedef struct map_t map_t;
typedef struct node_t node_t;

typedef struct h2_t h2_t;
typedef struct h2_cookie_t h2_cookie_t;

typedef void (*h2_resolve_t)(h2_cookie_t *cookie, h2_t *self, void *user);

typedef enum h2_digit_t {
    eDigitChar,
    eDigitShort,
    eDigitInt,
    eDigitLong,

    eDigitTotal
} h2_digit_t;

typedef enum h2_sign_t {
    eSignDefault,
    eSignUnsigned,
    eSignSigned,

    eSignTotal
} h2_sign_t;

typedef enum h2_link_t {
    eLinkExport,
    eLinkImport,
    eLinkModule,

    eLinkEntryCli,

    eLinkTotal
} h2_link_t;

typedef enum h2_visible_t {
    eVisiblePublic,
    eVisiblePrivate,

    eVisibleTotal
} h2_visible_t;

typedef enum h2_quals_t {
    eQualDefault = (0 << 0),
    eQualMutable = (1 << 0),
    eQualVolatile = (1 << 1),
    eQualAtomic = (1 << 2),
} h2_quals_t;

typedef enum sema2_tags_t {
    eSema2Values,
    eSema2Types,
    eSema2Procs,
    eSema2Modules,

    eSema2Total
} sema2_tags_t;

typedef struct h2_attrib_t {
    h2_link_t link; ///< the link type of the declaration
    h2_visible_t visible; ///< the visibility of the declaration

    const char *mangle; ///< override the mangle of the declaration
    const char *deprecated; ///< the reason for deprecation, or NULL if not deprecated
} h2_attrib_t;

typedef enum h2_kind_t {
    eHlir2Module,

    eHlir2TypeMeta,
    eHlir2TypeEmpty,
    eHlir2TypeUnit,
    eHlir2TypeBool,
    eHlir2TypeDigit,
    eHlir2TypeString,

    eHlir2Begin,
    eHlir2Error,

    eHlir2Total
} h2_kind_t;

typedef struct h2_t {
    h2_kind_t kind;
    const node_t *node;
    const h2_t *type;

    union {
        /* eHlir2Digit */
        mpz_t digitValue;

        /* eHlir2Bool */
        bool boolValue;

        /* eHlir2String */
        struct {
            const char *stringValue;
            size_t stringLength;
        };

        /* eHlir2Error */
        struct {
            const char *message;
            vector_t *trace;
        };
        
        /* any declaration */
        struct {
            const char *name; ///< the name of the declaration
            const h2_attrib_t *attrib; ///< the attributes of the declaration

            union {
                /* eHlir2TypeDigit */
                struct {
                    h2_digit_t digit;
                    h2_sign_t sign;
                };

                /* eHlir2Begin */
                struct {
                    void *user;
                    h2_resolve_t fnResolve;
                };

                /* eHlir2Module */
                struct {
                    h2_t *parent;
                    reports_t *reports;
                    map_t *tags;
                };
            };
        };
    };
} h2_t;

///
/// h2 error handling
///

h2_t *h2_error(const node_t *node, const char *message);

///
/// h2 type interface
///

/**
 * @brief the type of all types
 * 
 * @param node where this type was defined
 * @param name the name of the type
 * @return the type
 */
h2_t *h2_type_meta(const node_t *node, const char *name);

/**
 * @brief create an empty type, this is a type that has no values and can never be created in a well defined program
 * 
 * @param node where this type was defined
 * @param name the name of the type
 * @return an empty type
 */
h2_t *h2_type_empty(const node_t *node, const char *name);

/**
 * @brief create a unit type, this is a type that has only one value. equivilent to void
 * 
 * @param node where this type was defined
 * @param name the name of the type
 * @return a unit type
 */
h2_t *h2_type_unit(const node_t *node, const char *name);

/**
 * @brief create a bool type, this is a type that has only two values, true and false
 * 
 * @param node where this type was defined
 * @param name the name of the type
 * @return a bool type
 */
h2_t *h2_type_bool(const node_t *node, const char *name);

/**
 * @brief create a digit type
 * 
 * @param node where this type was defined
 * @param name the name of the type
 * @param digit the width of the digit
 * @param sign the sign of the digit
 * @return a digit type
 */
h2_t *h2_type_digit(const node_t *node, const char *name, h2_digit_t digit, h2_sign_t sign);

/**
 * @brief create a string type
 * 
 * @param node where this type was defined
 * @param name the name of the type
 * @return a string type
 */
h2_t *h2_type_string(const node_t *node, const char *name);

h2_t *h2_type_qualify(const node_t *node, const h2_t *type, h2_quals_t quals);


///
/// h2 expr interface
///

h2_t *h2_expr_empty(const node_t *node, const h2_t *type);
h2_t *h2_expr_void(const node_t *node, const h2_t *type);
h2_t *h2_expr_bool(const node_t *node, const h2_t *type, bool value);
h2_t *h2_expr_digit(const node_t *node, const h2_t *type, mpz_t value);

/**
 * @brief create a string expression
 * 
 * @param node a node to attach the expression to
 * @param type the type of the expression
 * @param value a null terminated string
 * @param length the length of the string not including the null terminator
 * @return h2_t* 
 */
h2_t *h2_expr_string(const node_t *node, const h2_t *type, const char *value, size_t length);

h2_t *h2_expr_load(const node_t *node, h2_t *expr);
h2_t *h2_expr_unary(const node_t *node, unary_t unary, h2_t *expr);
h2_t *h2_expr_binary(const node_t *node, const h2_t *type, binary_t binary, h2_t *lhs, h2_t *rhs);
h2_t *h2_expr_compare(const node_t *node, const h2_t *type, compare_t compare, h2_t *lhs, h2_t *rhs);

h2_t *h2_expr_call(const node_t *node, const h2_t *expr, vector_t *args);

///
/// h2 statement interface
///

/**
 * @brief create a block statement
 * 
 * @param node the location of the block statement
 * @param stmts the statements in the block
 * @return the block statement
 */
h2_t *h2_stmt_block(const node_t *node, vector_t *stmts);

/**
 * @brief create a return statement
 * 
 * @note this is only valid in a function
 * @note either @a value or @a type must not be NULL
 * @note if neither are null then the type of @a value must be the same as @a type
 * 
 * @param node the location of the return statement
 * @param type the type of the return statement, this must be the same as the return type of the function
 * @param value the value to return
 * @return the return statement
 */
h2_t *h2_stmt_return(const node_t *node, const h2_t *type, h2_t *value);


h2_t *h2_stmt_assign(const node_t *node, h2_t *dst, h2_t *src);
h2_t *h2_stmt_loop(const node_t *node, h2_t *cond, h2_t *body, h2_t *other);
h2_t *h2_stmt_branch(const node_t *node, h2_t *cond, h2_t *then, h2_t *other);

///
/// h2 decl interface
///

// delay the resolve of a declaration
void h2_resolve(h2_cookie_t *cookie, h2_t *decl);
h2_t *h2_decl_open(const node_t *node, const char *name, void *user, h2_kind_t expected, h2_resolve_t fnResolve);

h2_t *h2_open_global(const node_t *node, const char *name, const h2_t *type);
h2_t *h2_open_function(const node_t *node, const char *name, const h2_t *result, vector_t *params);

void h2_close_global(h2_t *self, h2_t *value);
void h2_close_function(h2_t *self, h2_t *body);

h2_t *h2_decl_param(const node_t *node, const char *name, const h2_t *type);
h2_t *h2_decl_local(const node_t *node, const char *name, const h2_t *type);
h2_t *h2_decl_global(const node_t *node, const char *name, const h2_t *type, h2_t *value);
h2_t *h2_decl_function(const node_t *node, const char *name, const h2_t *result, vector_t *params, h2_t *body);

///
/// various helpers
///

void h2_add_local(h2_t *self, h2_t *decl);
void h2_set_attrib(h2_t *self, const h2_attrib_t *attrib);

///
/// h2 sema interface
///

h2_t *h2_module_root(reports_t *reports, const node_t *node, const char *name, size_t decls, size_t *sizes);

/**
 * @brief create a new module
 * 
 * @param parent the parent module
 * @param node location of the module
 * @param name the name of the module
 * @param decls the number of declaration categories in the module
 * @param sizes the size of each declaration category
 * @return the module
 */
h2_t *h2_module(h2_t *parent, const node_t *node, const char *name, size_t decls, size_t *sizes);

/**
 * @brief recursively search for a declaration in a module
 * 
 * @param self the module
 * @param tag the declaration category
 * @param name the name of the declaration
 * @return the declaration or NULL if it does not exist
 */
h2_t *h2_module_get(h2_t *self, size_t tag, const char *name);

/**
 * @brief set a declaration in the current module
 * 
 * @note if the declaration already exists it will not be replaced
 * 
 * @param self the module
 * @param tag the declaration category
 * @param name the name of the declaration
 * @param value the value of the declaration
 * @return a previous declaration or NULL if it did not exist
 */
h2_t *h2_module_set(h2_t *self, size_t tag, const char *name, h2_t *value);

map_t *h2_module_tag(h2_t *self, size_t tag);

/**
 * @brief return a resolution cookie
 * 
 * @param self the module
 * @return the cookie
 */
h2_cookie_t *h2_module_cookie(h2_t *self);
