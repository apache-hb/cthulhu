#ifndef AST_H
#define AST_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct node_t node_t;

typedef enum {
    /* literals */
    NODE_DIGIT,
    NODE_STRING,
    NODE_NULL,

    /* expressions */
    NODE_UNARY,
    NODE_BINARY,
    NODE_TERNARY,
    NODE_FIELD,
    NODE_APPLY,
    NODE_SUBSCRIPT,
    NODE_CAST,
    NODE_NAME,
    NODE_LIST,

    /* statements */
    NODE_COMPOUND,
    NODE_RETURN,
    NODE_ASSIGN,
    NODE_BREAK,
    NODE_CONTINUE,
    NODE_ATTRIB,
    NODE_WHILE,
    NODE_FOR,

    /* decls */
    NODE_MINDECL,

    NODE_VAR,
    NODE_FUNC,
    NODE_PARAM,
    NODE_ALIAS,
    NODE_ITEM,
    NODE_INCLUDE,
    NODE_MODULE,
    NODE_ARG,

    NODE_MAXDECL,

    /* types */
    NODE_MINTYPE,
    
    NODE_QUALIFIED,
    NODE_TYPENAME,
    NODE_BUILTIN,
    NODE_POINTER,
    NODE_ARRAY,
    NODE_CLOSURE,
    NODE_RECORD,
    NODE_UNION,
    NODE_VARIANT,
    NODE_CASE,

    NODE_MAXTYPE
} node_type_t;

typedef enum {
    OP8,
    OP16,
    OP32,
    OP64,
    OP128,
    OP256,
    OP512
} opsize_t;

typedef struct nodes_t {
    size_t length;
    size_t size;
    struct node_t *data;
} nodes_t;

typedef struct path_t {
    size_t length;
    size_t size;
    char **data;
} path_t;

typedef struct node_t {
    node_type_t type;
    bool mut:1;
    bool exported:1;
    bool comptime:1;

    struct YYLTYPE *loc;

    union {
        char *text;

        struct attrib_t {
            path_t *path;
            nodes_t *args;
        } attrib;

        struct scope_t {
            path_t *path;
            nodes_t *decls;
        } scope;

        struct include_t {
            path_t *path;
            path_t *items;
        } include;

        struct decl_t {
            char *name;
            nodes_t *attribs;

            union {
                struct var_t {
                    node_t *names;
                    node_t *type;
                    node_t *init;
                } var;

                struct func_t {
                    nodes_t *params;
                    node_t *result;
                    node_t *body;
                } func;

                struct param_t {
                    node_t *type;
                    node_t *init;
                } param;

                struct variant_t {
                    node_t *type;
                    nodes_t *cases;
                } variant;

                struct case_t {
                    nodes_t *fields;
                    node_t *init;
                } vcase;

                struct while_t {
                    node_t *cond;
                    node_t *body;
                    node_t *tail;
                } loop;

                struct for_t {
                    node_t *names;
                    node_t *range;
                    node_t *body;
                    node_t *tail;
                } iter;

                node_t *expr;
                node_t *type;
                nodes_t *fields;
                nodes_t *opcodes;
            };
        } decl;

        struct branch_t {
            node_t *cond;
            node_t *body;
            node_t *next;
        } branch;

        struct digit_t {
            char *digit;
            char *suffix;
        } digit;

        struct unary_t {
            int op;
            node_t *expr;
        } unary;

        struct binary_t {
            int op;
            node_t *lhs;
            node_t *rhs;
        } binary;

        struct ternary_t {
            node_t *cond;
            node_t *lhs;
            node_t *rhs;
        } ternary;

        struct field_t {
            node_t *expr;
            char *field;
            bool indirect;
        } field;

        struct apply_t {
            node_t *expr;
            nodes_t *args;
        } apply;

        struct closure_t {
            node_t *result;
            nodes_t *args;
        } closure;

        struct array_t {
            node_t *type;
            node_t *size;
        } array;

        struct subscript_t {
            node_t *expr;
            node_t *index;
        } subscript;

        struct cast_t {
            node_t *expr;
            node_t *type;
        } cast;

        struct assign_t {
            node_t *lhs;
            node_t *rhs;
            int op;
        } assign;

        nodes_t *items;
        node_t *expr;
        nodes_t *stmts;
        path_t *qualified;
        node_t *pointer;
    };
} node_t;

nodes_t *empty_list(void);
nodes_t *list(node_t *init);
nodes_t *list_add(nodes_t *self, node_t *node);
nodes_t *list_push(nodes_t *self, node_t *node);
nodes_t *list_join(nodes_t *self, nodes_t *other);

path_t *empty_path(void);
path_t *path(char *init);
path_t *path_add(path_t *self, char *item);

/* decls */
node_t *exported(node_t *decl, bool exported);
node_t *comptime(node_t *decl, bool comptime);
node_t *mut(node_t *node);
node_t *var(node_t *names, node_t *type, node_t *init, bool mut);
node_t *func(char *name, nodes_t *params, node_t *result, node_t *body);
node_t *param(char *name, node_t *type, node_t *init);
node_t *alias(char *name, node_t *type);
node_t *record(char *name, nodes_t *items);
node_t *item(char *name, node_t *type);
node_t *include(path_t *path, path_t *items);
node_t *uniondecl(char *name, nodes_t *items);
node_t *scope(path_t *path, nodes_t *decls);
node_t *arg(char *name, node_t *expr);
node_t *attach(node_t *decl, nodes_t *attribs);
node_t *attribute(path_t *path, nodes_t *args);
node_t *nwhile(char *label, node_t *cond, node_t *body, node_t *tail);
node_t *nfor(char *label, node_t *names, node_t *range, node_t *body, node_t *tail);

/* statements */
node_t *result(node_t *expr);
node_t *compound(nodes_t *stmts);
node_t *assign(node_t *lhs, node_t *rhs, int op);
node_t *nbreak(char *label);
node_t *ncontinue();

/* types */
node_t *redirect(char *name);
node_t *qualified(path_t *path);
node_t *array(node_t *type, node_t *size);
node_t *closure(nodes_t *args, node_t *result);
node_t *pointer(node_t *type);
node_t *builtin(char *name);
node_t *bitfield(node_t *type, nodes_t *ranges);
node_t *variant(char *name, node_t *type, nodes_t *cases);
node_t *variantcase(char *name, nodes_t *fields, node_t *init);

/* expressions */
node_t *unary(int op, node_t *expr);
node_t *binary(int op, node_t *lhs, node_t *rhs);
node_t *ternary(node_t *cond, node_t *lhs, node_t *rhs);
node_t *field(node_t *expr, char *field, bool indirect);
node_t *apply(node_t *expr, nodes_t *args);
node_t *subscript(node_t *expr, node_t *index);
node_t *cast(node_t *expr, node_t *type);
node_t *name(char *text);
node_t *items(nodes_t *items);

/* constants */
node_t *string(char *text);
node_t *nil(void);
node_t *digit(char *text, char *suffix);
node_t *bitrange(node_t *front, node_t *back);

void dump_nodes(nodes_t *nodes, bool feed);
void dump_node(node_t *node);

#endif /* AST_H */
