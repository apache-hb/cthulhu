#pragma once

#include "cthulhu/ast/ast.h"
#include "cthulhu/ast/ops.h"

#include "type.h"

#include <gmp.h>

typedef enum {
    /* expressions */
    HLIR_DIGIT, /// a digit
    HLIR_NAME, /// a reference to a name
    HLIR_BINARY, /// a binary operation

    /* statements */
    HLIR_ASSIGN, /// an assignment

    /* declarations */
    HLIR_VALUE, /// a value
    HLIR_FUNCTION, /// a function
    HLIR_DECLARE, /// forward declaration

    HLIR_MODULE, /// a compilation unit

    HLIR_ERROR /// a compilation error
} hlir_type_t;

typedef struct hlir_t {
    hlir_type_t kind;
    const node_t *node;
    const type_t *type;

    union {
        /* a digit */
        mpz_t digit;

        /* a reference to a name */
        struct hlir_t *ident;

        /* a binary operation. lhs op rhs */
        struct {
            struct hlir_t *lhs;
            struct hlir_t *rhs;
            binary_t binary;
        };

        /* an assignment. lhs = rhs */
        struct {
            struct hlir_t *dst;
            struct hlir_t *src;
        };

        /* any named declaration */
        struct {
            /* the name of the declaration */
            const char *name;

            union {
                /* the type this is expected to take when complete */
                hlir_type_t expect;

                /* the value this evaluates to */
                struct hlir_t *value;

                /* the function this evaluates to */
                struct {
                    /* the body of the function, a list of statements */
                    vector_t *body;
                };
            };
        };

        /* HLIR_MODULE */
        struct {
            /* the module name */
            const char *mod;

            /* all symbols this module imports */
            vector_t *imports;

            /* all variables defined in this module */
            vector_t *globals;

            /* all functions defined in this module */
            vector_t *defines;
        };
    };
} hlir_t;

hlir_t *hlir_digit(const node_t *node, const type_t *type, mpz_t digit);
hlir_t *hlir_int(const node_t *node, const type_t *type, uintmax_t digit);

hlir_t *hlir_name(const node_t *node, const type_t *type, hlir_t *hlir);

hlir_t *hlir_binary(const node_t *node, const type_t *type, hlir_t *lhs, hlir_t *rhs, binary_t op);

hlir_t *hlir_assign(const node_t *node, hlir_t *dst, hlir_t *src);

hlir_t *hlir_value(const node_t *node, const type_t *type, const char *name, hlir_t *value);
hlir_t *hlir_function(const node_t *node, const type_t *type, const char *name, vector_t *body);

hlir_t *hlir_declare(const node_t *node, const char *name, hlir_type_t expect);

hlir_t *hlir_module(const node_t *node, const char *mod, vector_t *imports, vector_t *globals, vector_t *defines);
hlir_t *hlir_error(const node_t *node, const type_t *type);
