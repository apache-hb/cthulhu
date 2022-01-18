#pragma once

#include "cthulhu/ast/ast.h"
#include "cthulhu/ast/ops.h"

#include "type.h"

#include <gmp.h>

typedef enum {
    /* expressions */
    HLIR_DIGIT, /// a digit

    /* declarations */
    HLIR_VALUE, /// a value
    HLIR_FUNCTION, /// a function
    HLIR_DECLARE, /// forward declaration

    HLIR_MODULE /// a compilation unit
} hlir_type_t;

typedef struct hlir_t {
    hlir_type_t kind;
    node_t *node;
    const type_t *type;

    union {
        /* a digit */
        mpz_t digit;

        /* any named declaration */
        struct {
            /* the name of the declaration */
            const char *name;

            union {
                /* the type this is expected to take when complete */
                hlir_type_t expect;

                /* the value this evaluates to */
                struct hlir_t *value;
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

hlir_t *hlir_digit(node_t *node, const type_t *type, mpz_t digit);
hlir_t *hlir_int(node_t *node, const type_t *type, uintmax_t digit);

hlir_t *hlir_value(node_t *node, const type_t *type, const char *name, hlir_t *value);

hlir_t *hlir_declare(node_t *node, const char *name, hlir_type_t expect);

hlir_t *hlir_module(node_t *node, const char *mod, vector_t *imports, vector_t *globals, vector_t *defines);
