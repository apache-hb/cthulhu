#pragma once

#include "cthulhu/tree/ops.h"

#include <stdbool.h>
#include <gmp.h>

typedef struct reports_t reports_t;

typedef struct vector_t vector_t;
typedef struct map_t map_t;
typedef struct node_t node_t;

typedef struct tree_t tree_t;
typedef struct cookie_t cookie_t;

typedef void (*resolve_t)(cookie_t *cookie, tree_t *sema, tree_t *self, void *user);

typedef enum sema_tags_t {
    eSema2Values,
    eSema2Types,
    eSema2Procs,
    eSema2Modules,

    eSema2Total
} sema_tags_t;

typedef struct attribs_t {
    tree_link_t link; ///< the link type of the declaration
    visibility_t visibility; ///< the visibility of the declaration

    const char *mangle; ///< override the mangle of the declaration
    const char *deprecated; ///< the reason for deprecation, or NULL if not deprecated
} attribs_t;

typedef enum tree_kind_t {
#define TREE_KIND(ID, NAME) ID,
#include "cthulhu/tree/tree.inc"

    eTreeTotal
} tree_kind_t;

typedef struct cookie_t {
    reports_t *reports;
    vector_t *stack;
} cookie_t;

typedef struct tree_resolve_info_t {
    tree_t *sema;
    void *user;

    resolve_t fnResolve;
} tree_resolve_info_t;

typedef struct tree_storage_t {
    const tree_t *storage;
    size_t size;
    quals_t quals;
} tree_storage_t;

typedef struct tree_t {
    tree_kind_t kind;
    const node_t *node;
    const tree_t *type;

    union {
        /* eTreeDigit */
        mpz_t digitValue;

        /* eTreeBool */
        bool boolValue;

        /* eTreeString */
        struct {
            const char *stringValue;
            size_t stringLength;
        };

        /* eTreeExprLoad */
        tree_t *load;

        /* eTreeExprRef */
        tree_t *expr;

        /* eTreeExprCast */
        tree_t *cast;

        /* eTreeExprUnary */
        struct {
            unary_t unary;
            tree_t *operand;
        };

        /* eTreeExprBinary|eTreeExprCompare */
        struct {
            union {
                binary_t binary;
                compare_t compare;
            };

            tree_t *lhs;
            tree_t *rhs;
        };

        /* eTreeExprCall */
        struct {
            const tree_t *callee;
            vector_t *args;
        };

        /* eTreeError */
        const char *message;

        /* eTreeStmtBlock */
        vector_t *stmts;

        /* eTreeStmtReturn */
        const tree_t *value;

        /* eTreeStmtAssign */
        struct {
            tree_t *dst;
            tree_t *src;
        };

        /* eTreeStmtLoop|eTreeStmtBranch */
        struct {
            tree_t *cond;
            tree_t *then;
            tree_t *other;
        };

        /* eTreeExprField */
        struct {
            tree_t *object;
            tree_t *field;
        };

        /* any declaration */
        struct {
            const char *name; ///< the name of the declaration
            const attribs_t *attrib; ///< the attributes of the declaration
            const tree_resolve_info_t *resolve; ///< the resolve configuration of the declaration, NULL if resolved
            quals_t quals;

            union {
                /* eTreeTypePointer|eTreeTypeReference */
                struct {
                    const tree_t *ptr;
                    size_t length;
                };

                /* eTreeTypeDigit */
                struct {
                    digit_t digit;
                    sign_t sign;
                };

                /* eTreeTypeStruct */
                vector_t *fields;

                struct {
                    vector_t *params;

                    union {
                        /* eTreeDeclFunction */
                        struct {
                            vector_t *locals;
                            tree_t *body;
                        };

                        /* eTreeTypeClosure */
                        struct {
                            const tree_t *result;
                            arity_t arity;
                        };
                    };
                };

                /* eTreeDeclGlobal|eTreeDeclLocal */
                struct {
                    tree_storage_t storage;

                    tree_t *initial;
                };

                /* eTreeDeclModule */
                struct {
                    tree_t *parent;
                    cookie_t *cookie;

                    reports_t *reports;
                    vector_t *tags; ///< vector_t<map_t<const char*, void*>*>
                    map_t *extra;
                };
            };
        };
    };
} tree_t;

///
/// tree error handling
///

tree_t *tree_error(const node_t *node, const char *message, ...);
tree_t *tree_raise(const node_t *node, reports_t *reports, const char *message, ...);

///
/// tree type interface
///

/**
 * @brief create an empty type, this is a type that has no values and can never be created in a well defined program
 *
 * @param node where this type was defined
 * @param name the name of the type
 * @return an empty type
 */
tree_t *tree_type_empty(const node_t *node, const char *name);

/**
 * @brief create a unit type, this is a type that has only one value. equivilent to void
 *
 * @param node where this type was defined
 * @param name the name of the type
 * @return a unit type
 */
tree_t *tree_type_unit(const node_t *node, const char *name);

/**
 * @brief create a bool type, this is a type that has only two values, true and false
 *
 * @param node where this type was defined
 * @param name the name of the type
 * @return a bool type
 */
tree_t *tree_type_bool(const node_t *node, const char *name, quals_t quals);

/**
 * @brief create a digit type
 *
 * @param node where this type was defined
 * @param name the name of the type
 * @param digit the width of the digit
 * @param sign the sign of the digit
 * @return a digit type
 */
tree_t *tree_type_digit(const node_t *node, const char *name, digit_t digit, sign_t sign, quals_t quals);

/**
 * @brief create a function pointer type
 *
 * @param node where this type was defined
 * @param name the name of the type
 * @param result return type of the function
 * @param params the parameters of the function
 * @param arity does this function have variadic arguments
 * @return tree_t* the function pointer type
 */
tree_t *tree_type_closure(const node_t *node, const char *name, const tree_t *result, vector_t *params, arity_t arity);

/**
 * @brief create a pointer type
 *
 * @param node where this type was defined
 * @param name the name of the type
 * @param pointer the type that this pointer points to
 * @return tree_t* the pointer type
 */
tree_t *tree_type_pointer(const node_t *node, const char *name, const tree_t *pointer, size_t length);

/**
 * @brief create a reference type
 *
 * @param node the location of the reference
 * @param name the name of the type
 * @param reference the type that this reference references
 * @return tree_t* the reference type
 */
tree_t *tree_type_reference(const node_t *node, const char *name, const tree_t *reference);

///
/// tree expr interface
///

tree_t *tree_expr_empty(const node_t *node, const tree_t *type);
tree_t *tree_expr_unit(const node_t *node, const tree_t *type);
tree_t *tree_expr_bool(const node_t *node, const tree_t *type, bool value);
tree_t *tree_expr_digit(const node_t *node, const tree_t *type, const mpz_t value);

/**
 * @brief create a string expression
 *
 * @param node a node to attach the expression to
 * @param type the type of the expression
 * @param value a null terminated string
 * @param length the length of the string not including the null terminator
 * @return tree_t*
 */
tree_t *tree_expr_string(const node_t *node, const tree_t *type, const char *value, size_t length);

tree_t *tree_expr_cast(const node_t *node, const tree_t *type, tree_t *expr);

/**
 * @brief load a value from a pointer or storage
 *
 * @param node the location of the load
 * @param expr the pointer or storage to load from
 * @return tree_t* the loaded value
 */
tree_t *tree_expr_load(const node_t *node, tree_t *expr);

/**
 * @brief create a reference to an object
 *
 * @param node the location of the reference
 * @param expr the object to reference
 * @return tree_t*
 */
tree_t *tree_expr_ref(const node_t *node, tree_t *expr);

/**
 * @brief create a reference to an object
 *
 * @param node the location of the reference
 * @param expr the object to reference
 * @return tree_t* the reference
 */
tree_t *tree_expr_address(const node_t *node, tree_t *expr);

tree_t *tree_expr_unary(const node_t *node, unary_t unary, tree_t *expr);
tree_t *tree_expr_binary(const node_t *node, const tree_t *type, binary_t binary, tree_t *lhs, tree_t *rhs);
tree_t *tree_expr_compare(const node_t *node, const tree_t *type, compare_t compare, tree_t *lhs, tree_t *rhs);

tree_t *tree_expr_field(const node_t *node, tree_t *object, tree_t *field);

tree_t *tree_expr_call(const node_t *node, const tree_t *callee, vector_t *args);

///
/// tree statement interface
///

/**
 * @brief create a block statement
 *
 * @param node the location of the block statement
 * @param stmts the statements in the block
 * @return the block statement
 */
tree_t *tree_stmt_block(const node_t *node, vector_t *stmts);

/**
 * @brief create a return statement
 *
 * @note this is only valid in a function
 *
 * @param node the location of the return statement
 * @param value the value to return
 * @return the return statement
 */
tree_t *tree_stmt_return(const node_t *node, const tree_t *value);

tree_t *tree_stmt_assign(const node_t *node, tree_t *dst, tree_t *src);
tree_t *tree_stmt_loop(const node_t *node, tree_t *cond, tree_t *body, tree_t *other);
tree_t *tree_stmt_branch(const node_t *node, tree_t *cond, tree_t *then, tree_t *other);

///
/// tree decl interface
///

// delay the resolve of a declaration
tree_t *tree_resolve(cookie_t *cookie, const tree_t *decl);
void tree_set_storage(tree_t *self, tree_storage_t storage);

tree_t *tree_open_decl(const node_t *node, const char *name, tree_resolve_info_t resolve);
void tree_close_decl(tree_t *self, const tree_t *kind);

tree_t *tree_decl_global(
    const node_t *node, const char *name,
    tree_storage_t storage, const tree_t *type, tree_t *initial
);

tree_t *tree_open_global(
    const node_t *node, const char *name,
    const tree_t *type, tree_resolve_info_t resolve
);

void tree_close_global(tree_t *self, tree_t *value);

///
/// function decls
///

tree_t *tree_decl_function(
    const node_t *node, const char *name, const tree_t *signature,
    vector_t *params, vector_t *locals, tree_t *body
);

tree_t *tree_open_function(
    const node_t *node, const char *name,
    const tree_t *signature, tree_resolve_info_t resolve
);

void tree_close_function(tree_t *self, tree_t *body);

///
/// struct decls
///

tree_t *tree_decl_struct(const node_t *node, const char *name, vector_t *fields);
tree_t *tree_open_struct(const node_t *node, const char *name, tree_resolve_info_t resolve);
void tree_close_struct(tree_t *self, vector_t *fields);

///
/// other decls
///

tree_t *tree_decl_param(const node_t *node, const char *name, const tree_t *type);
tree_t *tree_decl_field(const node_t *node, const char *name, const tree_t *type);
tree_t *tree_decl_local(const node_t *node, const char *name, tree_storage_t storage, const tree_t *type);

///
/// various helpers
///

void tree_add_local(tree_t *self, tree_t *decl);
void tree_add_param(tree_t *self, tree_t *decl);
void tree_set_attrib(tree_t *self, const attribs_t *attrib);

tree_t *tree_alias(const tree_t *tree, const char *name);

///
/// tree sema interface
///

// only declarations placed in the tags eSema2Values, eSema2Types, eSema2Procs, and eSema2Modules
// will be emitted, they are also required to be valid tree_t objects
// any custom slots can contain any data, but they will not be emitted

tree_t *tree_module_root(reports_t *reports, cookie_t *cookie, const node_t *node, const char *name, size_t decls, const size_t *sizes);

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
tree_t *tree_module(tree_t *parent, const node_t *node, const char *name, size_t decls, const size_t *sizes);

/**
 * @brief recursively search for a declaration in a module
 *
 * @param self the module
 * @param tag the declaration category
 * @param name the name of the declaration
 * @return the declaration or NULL if it does not exist
 */
void *tree_module_get(tree_t *self, size_t tag, const char *name);

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
void *tree_module_set(tree_t *self, size_t tag, const char *name, void *value);

map_t *tree_module_tag(const tree_t *self, size_t tag);

/**
 * @brief return a resolution cookie
 *
 * @param self the module
 * @return the cookie
 */
cookie_t *tree_get_cookie(tree_t *sema);
