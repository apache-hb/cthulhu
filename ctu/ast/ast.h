#pragma once

#include <stdint.h>
#include <gmp.h>

#include "scanner.h"
#include "ctu/types/type.h"

#include "ctu/util/util.h"

typedef enum {
    /**
     * expressions
     */
    AST_DIGIT,
    AST_BOOL,
    AST_STRING,
    AST_UNARY,
    AST_BINARY,
    AST_CALL,
    AST_CAST,
    AST_ACCESS,
    AST_INDEX,

    /**
     * statements
     */
    AST_STMTS,
    AST_RETURN,
    AST_BRANCH,
    AST_ASSIGN,
    AST_WHILE,

    /**
     * declarations
     */
    AST_DECL_FUNC,
    AST_DECL_VAR,
    AST_DECL_PARAM,
    AST_DECL_STRUCT,
    AST_DECL_FIELD,
    AST_DECL_IMPORT,
    AST_ATTRIB,

    /**
     * types
     */
    AST_SYMBOL,
    AST_PTR,
    AST_MUT,
    AST_ARRAY,

    /**
     * implementation details
     */
    AST_TYPE,
    AST_ROOT,
    AST_NOOP
} ast_t;

typedef enum {
    UNARY_ABS, /* +expr */
    UNARY_NEG, /* -expr */

    UNARY_REF, /* &expr */
    UNARY_DEREF, /* *expr */

    UNARY_TRY, /* expr? */
} unary_t;

typedef enum {
    BINARY_ADD, /* expr + expr */
    BINARY_SUB, /* expr - expr */
    BINARY_MUL, /* expr * expr */
    BINARY_DIV, /* expr / expr */
    BINARY_REM, /* expr % expr */

    BINARY_LT, /* expr < expr */
    BINARY_LTE, /* expr <= expr */
    BINARY_GT, /* expr > expr */
    BINARY_GTE, /* expr >= expr */

    BINARY_EQ, /* expr == expr */
    BINARY_NEQ, /* expr != expr */

    BINARY_SHL, /* expr << expr */
    BINARY_SHR, /* expr >> expr */
    BINARY_XOR, /* expr ^ expr */

    BINARY_AND, /* expr && expr */
    BINARY_OR, /* expr || epxr */

    BINARY_BITAND, /* expr & expr */
    BINARY_BITOR, /* expr | expr */
} binary_t;

typedef enum {
    ATTR_USED = (1 << 0), /* is this used */
    ATTR_EXPORT = (1 << 1), /* is this exported */
    ATTR_COMPILE = (1 << 2), /* is this compiled */
    ATTR_MUTABLE = (1 << 3), /* is this mutable */
    ATTR_IMPLICIT = (1 << 4), /* is this implicit */
    ATTR_INTEROP = (1 << 5), /* is this an external function */
} attrib_t;

/**
 * is any binary math operation
 */
bool is_math_op(binary_t op);

/**
 * is any binary comparison operator, not including equality
 */
bool is_comparison_op(binary_t op);

/**
 * is any binary equality operator
 */
bool is_equality_op(binary_t op);

#define NOT_LOCAL SIZE_MAX

/**
 * define this to either struct or union
 * for debugging purposes
 */
#define AST_UNION union

/**
 * we need string interning at some point
 */
typedef struct node_t {
    /**
     * the type of this node 
     */
    ast_t kind;

    /**
     * the scanner that produced this node
     */
    scanner_t *scanner;

    /**
     * where in the scanner this node came from
     */
    where_t where;

    /* attribute flags */
    attrib_t attribs;

    /**
     * the type of this nodes expression in its current context
     */
    type_t *typeof;

    /* own local index, or NOT_LOCAL if its not local */
    size_t local;

    AST_UNION {
        /* AST_SYMBOL */
        /** @var list_t<char*> */
        list_t *ident;

        /* AST_DECL_IMPORT */
        /** @var list_t<char*> */
        list_t *path;

        /* AST_ROOT */
        struct {
            /** @var list_t<node_t> */
            list_t *imports;

            /** @var list_t<node_t> */
            list_t *decls;
        };

        /* AST_ACCESS */
        struct {
            /* the object to access */
            struct node_t *target;

            /* the field to access */
            char *field;

            /* is the access direct or indirect */
            bool indirect;
        };

        /* AST_DIGIT */
        struct {
            /* actual number */
            mpz_t num;

            /* true if signed, false if unsigned */
            bool sign;
            
            /* the underlying integer type */
            integer_t integer;
        };

        /* AST_BOOL */
        bool boolean;

        /* AST_STMTS */
        /** @var list_t<node_t*> */
        list_t *stmts;

        /* AST_PTR */
        struct node_t *ptr;

        struct {
            /* AST_UNARY */
            unary_t unary;

            /* AST_RETURN */
            struct node_t *expr;

            /* part of a decorator */
            list_t *attr;

            AST_UNION {
                /* AST_CAST */
                struct node_t *cast;

                /* AST_CALL */
                /** @var list_t<node_t*> */
                list_t *args;

                struct node_t *index;
            };
        };

        /* AST_BINARY */
        struct {
            binary_t binary;
            struct node_t *lhs;
            struct node_t *rhs;
        };

        /* AST_ASSIGN */
        struct {
            struct node_t *dst;
            struct node_t *src;
        };

        /* AST_BRANCH */
        struct {
            struct node_t *cond;
            struct node_t *branch;

            /* AST_WHILE, AST_MUT */
            struct node_t *next;
        };

        /* AST_ARRAY */
        struct {
            /* array of item */
            struct node_t *of;
            
            /** 
             * the number of items in an array, 
             * if NULL then array is unbounded 
             */
            struct node_t *size;
        };

        /* AST_DECL */
        struct {
            AST_UNION {
                /**
                 * all declarations have names
                 */
                char *name;

                /* AST_TYPE */
                const char *nameof;
                
                /**
                 * string is here because strings are
                 * stored in a global array, not inline
                 * so we need to know where in the array
                 * they are, we use `local` to find that
                 */
                char *string;
            };

            /**
             * a semantic context to evaluate this decl inside of
             * actually a sema_t* defined in sema.c
             */
            void *ctx;

            /* decorators applied to this declaration */
            list_t *decorate;

            AST_UNION {
                /* AST_DECL_STRUCT */
                /** @var list_t<node_t*> */
                list_t *fields;

                /* AST_DECL_FUNC */
                struct {
                    /** @var list_t<node_t*> */
                    list_t *params;

                    struct node_t *result;
                    struct node_t *body;

                    /* total number of local variables */
                    size_t locals;
                };

                /* AST_DECL_VAR */
                struct {
                    /* AST_DECL_PARAM, AST_DECL_FIELD */
                    struct node_t *type;
                    struct node_t *init;
                };
            };
        };
    };
} node_t;

/**
 * query information
 */

/**
 * get the name of a declaration node
 * do not call on something that isnt a declaration
 */
const char *get_decl_name(node_t *node);

/**
 * get the name of a resolved type
 * do not call with symbols
 */
const char *get_resolved_name(node_t *node);

const char *get_field_name(node_t *node);

bool is_deref(node_t *expr);

bool is_access(node_t *expr);

bool is_symbol(node_t *it);

bool is_index(node_t *expr);

/**
 * get the type of a node
 * returns an unresolved type if the type hasnt been resolved
 */
type_t *get_type(node_t *node);

type_t *get_resolved_type(node_t *node);

/**
 * get the type of a node or NULL
 */
type_t *raw_type(node_t *node);

/**
 * get all statements in a list
 */
list_t *get_stmts(node_t *node);

/**
 * is this name the discarded name `$`
 */
bool is_discard_name(const char *name);

/**
 * is a decl exported
 */
bool is_exported(node_t *node);

/**
 * modify nodes
 */

/**
 * mark a node as implicitly generated
 * used in the semantic state to generate implicit casts
 */
node_t *make_implicit(node_t *node);

/**
 * mark a node as exported
 */
node_t *make_exported(node_t *node);

/**
 * mark a node as used so its not discarded
 */
void mark_used(node_t *node);
bool is_used(node_t *node);
bool is_mut(node_t *node);
void mark_interop(node_t *node);
bool is_interop(node_t *node);

/**
 * node creation
 */

node_t *ast_digit(scanner_t *scanner, where_t where, char *digit, int base);
node_t *ast_bool(scanner_t *scanner, where_t where, bool boolean);
node_t *ast_string(scanner_t *scanner, where_t where, char *string);

node_t *ast_unary(scanner_t *scanner, where_t where, unary_t unary, node_t *expr);
node_t *ast_binary(scanner_t *scanner, where_t where, binary_t binary, node_t *lhs, node_t *rhs);
node_t *ast_call(scanner_t *scanner, where_t where, node_t *body, list_t *args);
node_t *ast_cast(scanner_t *scanner, where_t where, node_t *expr, node_t *cast);
node_t *ast_access(scanner_t *scanner, where_t where, node_t *expr, char *name, bool indirect);

node_t *ast_stmts(scanner_t *scanner, where_t where, list_t *stmts);
node_t *ast_return(scanner_t *scanner, where_t where, node_t *expr);
node_t *ast_branch(scanner_t *scanner, where_t where, node_t *cond, node_t *branch);
node_t *add_branch(node_t *branch, node_t *next);
node_t *ast_assign(scanner_t *scanner, where_t where, node_t *dst, node_t *src);
node_t *ast_while(scanner_t *scanner, where_t where, node_t *cond, node_t *body);

node_t *ast_symbol(scanner_t *scanner, where_t where, list_t *text);
node_t *ast_pointer(scanner_t *scanner, where_t where, node_t *ptr);
node_t *ast_mut(scanner_t *scanner, where_t where, node_t *it);
node_t *ast_array(scanner_t *scanner, where_t where, node_t *of, node_t *size);

node_t *ast_decl_func(
    scanner_t *scanner, where_t where, 
    char *name, list_t *params,
    node_t *result, node_t *body
);
node_t *ast_decl_param(scanner_t *scanner, where_t where, char *name, node_t *type);
node_t *ast_decl_var(scanner_t *scanner, where_t where, bool mut, char *name, node_t *type, node_t *init);
node_t *ast_decl_struct(scanner_t *scanner, where_t where, char *name, list_t *fields);
node_t *ast_field(scanner_t *scanner, where_t where, char *name, node_t *type);

node_t *ast_import(scanner_t *scanner, where_t where, list_t *path);

node_t *ast_root(scanner_t *scanner, list_t *imports, list_t *decls);

/**
 * create a builtin type
 */
node_t *ast_type(const char *name);

node_t *ast_attribs(node_t *decl, attrib_t attribs, list_t *decorate);

node_t *ast_attrib(scanner_t *scanner, where_t where, list_t *name, list_t *args);
node_t *ast_noop(void);
node_t *ast_index(scanner_t *scanner, where_t where, node_t *expr, node_t *index);
