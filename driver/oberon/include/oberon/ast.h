#pragma once

#include "scan/node.h"

#include "std/vector.h"

#include <gmp.h>

typedef struct obr_t obr_t;

///
/// symbols
///

typedef enum obr_visibility_t {
    eObrVisPrivate,         ///< default (no suffix)
    eObrVisPublic,          ///< `*` suffix
    eObrVisPublicReadOnly   ///< `-` suffix
} obr_visibility_t;

typedef struct obr_symbol_t {
    scan_t *scan;
    where_t where;

    obr_visibility_t visibility;
    char *name;
} obr_symbol_t;

///
/// ast
///

typedef enum obr_kind_t {
    eObrTypeName,
    eObrTypeQual,

    eObrDeclVar,
    eObrDeclConst,
    eObrDeclProcedure,

    eObrModule,
    eObrImport
} obr_kind_t;

typedef struct obr_t {
    obr_kind_t kind;
    node_t *node;

    union {
        struct {
            char *name;
            obr_visibility_t visibility;

            union {
                /* eObrModule */
                struct {
                    vector_t *imports;
                    vector_t *decls;
                };

                /* eObrDeclVar */
                struct {
                    obr_t *type;

                    /* eObrDeclConst */
                    obr_t *value;
                };

                /* eObrImport|eObrTypeQual */
                char *symbol;
            };
        };
    };
} obr_t;

obr_t *obr_module(scan_t *scan, where_t where, char *name, char *end, vector_t *imports, vector_t *decls);
obr_t *obr_import(scan_t *scan, where_t where, char *name, char *symbol);

obr_t *obr_decl_var(scan_t *scan, where_t where, obr_symbol_t *symbol, obr_t *type);
obr_t *obr_decl_const(scan_t *scan, where_t where, obr_symbol_t *symbol, obr_t *type, obr_t *value);
obr_t *obr_decl_procedure(scan_t *scan, where_t where, obr_symbol_t *symbol, char *end);

obr_t *obr_type_name(scan_t *scan, where_t where, char *symbol);
obr_t *obr_type_qual(scan_t *scan, where_t where, char *name, char *symbol);

/* partial symbols */

obr_symbol_t *obr_symbol(scan_t *scan, where_t where, char *name, obr_visibility_t visibility);

vector_t *obr_expand_vars(vector_t *symbols, obr_t *type);
