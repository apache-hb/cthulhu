#ifndef AST_H
#define AST_H

#include <stdint.h>
#include <stddef.h>

typedef struct node_t node_t;
typedef struct nodes_t nodes_t;

typedef enum {
    /* literals */
    NODE_DIGIT,
    NODE_STRING,
    NODE_BOOL,

    /* expressions */
    NODE_UNARY,
    NODE_BINARY,
    NODE_TERNARY,
    NODE_CALL,
    NODE_NAME,
    NODE_ACCESS,

    /* statements */
    NODE_COMPOUND,
    NODE_RETURN,
    NODE_ASSIGN,
    NODE_BRANCH,
    NODE_WHILE,
    NODE_BREAK,
    NODE_CONTINUE,

    /* decls */
    NODE_FUNC,
    NODE_VAR,
    NODE_PARAM,
    NODE_RECORD,

    /* types */
    NODE_TYPENAME,
    NODE_POINTER,
    NODE_BUILTIN_TYPE,
    NODE_CLOSURE
} node_kind_t;

typedef enum {
    UNARY_ABS,
    UNARY_NEG,
    UNARY_REF,
    UNARY_DEREF,
    UNARY_NOT
} unary_op_t;

typedef enum {
    BINARY_ADD,
    BINARY_SUB,
    BINARY_DIV,
    BINARY_MUL,
    BINARY_REM
} binary_op_t;

typedef struct nodes_t {
    size_t length;
    size_t size;
    struct node_t *data;
} nodes_t;

typedef struct node_t {
    /* the kind of this node */
    node_kind_t kind;
    /* nearly everything is mutable or not */
    int mut;

    union {
        char *digit;
        char *name;
        char *text;
        int boolean;

        struct decl_t {
            char *name;

            /* NODE_PARAM doesnt care about this but everything else does */
            int exported;

            union {
                struct func_t {
                    nodes_t *params;
                    node_t *result;
                    node_t *body;
                } func;

                /* param type */
                node_t *param;

                struct var_t {
                    node_t *type;
                    node_t *init;
                } var;

                nodes_t *fields;
            };
        } decl;

        struct branch_t {
            node_t *cond;
            node_t *body;
            node_t *next;
        } branch;

        struct loop_t {
            node_t *cond;
            node_t *body;
        } loop;

        /* function signature */
        struct closure_t {
            nodes_t *args;
            node_t *result;
        } closure;

        /* pointer */
        node_t *type;

        struct unary_t {
            unary_op_t op;
            node_t *expr;
        } unary;

        struct binary_t {
            binary_op_t op;
            node_t *lhs;
            node_t *rhs;
        } binary;

        struct ternary_t {
            node_t *cond;
            node_t *yes;
            node_t *no;
        } ternary;

        struct call_t {
            node_t *body;
            nodes_t *args;
        } call;

        struct assign_t {
            node_t *old;
            node_t *expr;
        } assign;

        struct access_t {
            node_t *expr;
            char *field;
        } access;

        node_t *expr;
        nodes_t *compound;
    };
} node_t;

nodes_t*
empty_node_list(void);

nodes_t*
new_node_list(node_t *init);

nodes_t*
node_append(nodes_t *self, node_t *node);

nodes_t*
node_replace(nodes_t *self, size_t idx, node_t *node);

node_t*
set_exported(node_t *decl, int exported);

node_t*
new_digit(char *digit);

node_t*
new_access(node_t *expr, char *field);

node_t*
new_unary(unary_op_t op, node_t *expr);

node_t*
new_binary(binary_op_t op, node_t *lhs, node_t *rhs);

node_t*
new_ternary(node_t *cond, node_t *yes, node_t *no);

node_t*
new_call(node_t *body, nodes_t *args);

node_t*
new_func(char *name, nodes_t *params, node_t *result, node_t *body);

node_t*
new_param(char *name, node_t *type);

node_t*
new_return(node_t *expr);

node_t*
new_name(char *name);

node_t*
new_typename(char *name);

node_t*
new_builtin_type(const char *name);

node_t*
new_var(char *name, node_t *type, node_t *init, int mut);

node_t*
new_string(char *text);

node_t*
new_assign(node_t *old, node_t *it);

node_t*
new_bool(int val);

node_t*
new_array(node_t *type, node_t *size);

node_t*
new_pointer(node_t *type);

node_t*
new_compound(nodes_t *nodes);

node_t*
new_closure(nodes_t *args, node_t *result);

node_t*
new_branch(node_t *cond, node_t *body, node_t *next);

node_t*
new_record(char *name, nodes_t *fields);

node_t*
new_while(node_t *cond, node_t *body);

node_t*
new_break();

node_t*
new_continue();

node_t*
new_mut(node_t *node);

void
dump_node(node_t *node);

#endif /* AST_H */
