#pragma once

#include "cthulhu/ast/ops.h"
#include "cthulhu/ast/ast.h"
#include "cthulhu/data/value.h"

#include "attribs.h"

typedef enum {
    HLIR_LITERAL,

    HLIR_NAME,

    HLIR_BINARY,
    HLIR_COMPARE,

    HLIR_CALL,

    HLIR_STMTS,
    HLIR_BRANCH,
    HLIR_LOOP,
    HLIR_ASSIGN,

    HLIR_FORWARD,
    HLIR_FUNCTION,
    HLIR_VALUE,
    HLIR_MODULE,

    HLIR_ERROR
} hlir_type_t;

typedef struct hlir_t {
    hlir_type_t type; // the type of this node
    const node_t *node; // the node span of this hlir
    const type_t *of; // the type this hlir evaluates to

    union {
        struct hlir_t *read;

        value_t *literal;

        vector_t *stmts;

        struct {
            struct hlir_t *call;
            vector_t *args;
        };

        struct {
            struct hlir_t *lhs;
            struct hlir_t *rhs;

            union {
                binary_t binary;
                compare_t compare;
            };
        };

        struct {
            struct hlir_t *dst;
            struct hlir_t *src;
        };

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

const type_t *typeof_hlir(const hlir_t *self);

hlir_t *hlir_error(const node_t *node, const char *error);

hlir_t *hlir_literal(const node_t *node, value_t *value);
hlir_t *hlir_name(const node_t *node, hlir_t *read);

hlir_t *hlir_binary(const node_t *node, const type_t *type, binary_t binary, hlir_t *lhs, hlir_t *rhs);
hlir_t *hlir_compare(const node_t *node, const type_t *type, compare_t compare, hlir_t *lhs, hlir_t *rhs);

hlir_t *hlir_call(const node_t *node, hlir_t *call, vector_t *args);

hlir_t *hlir_stmts(const node_t *node, vector_t *stmts);
hlir_t *hlir_branch(const node_t *node, hlir_t *cond, hlir_t *then, hlir_t *other);
hlir_t *hlir_loop(const node_t *node, hlir_t *cond, hlir_t *body, hlir_t *other);
hlir_t *hlir_assign(const node_t *node, hlir_t *dst, hlir_t *src);

hlir_t *hlir_new_function(const node_t *node, const char *name, const type_t *type);
void hlir_add_local(hlir_t *self, hlir_t *local);
void hlir_build_function(hlir_t *self, hlir_t *body);

hlir_t *hlir_new_value(const node_t *node, const char *name, const type_t *type);
void hlir_build_value(hlir_t *self, hlir_t *value);

hlir_t *hlir_value(const node_t *node, const char *name, const type_t *type, hlir_t *value);

hlir_t *hlir_new_module(const node_t *node, const char *name);

void hlir_set_attributes(hlir_t *self, const hlir_attributes_t *attributes);
bool hlir_is_imported(const hlir_t *self);

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
