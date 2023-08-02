#pragma once

#include "scan/node.h"

#include "std/vector.h"

#include <gmp.h>

typedef struct obr_t obr_t;

typedef enum obr_kind_t {
    eObrTypeName,
    eObrTypeQual,

    eObrDeclGlobal,

    eObrModule,
    eObrImport
} obr_kind_t;

typedef struct obr_t {
    obr_kind_t kind;
    node_t *node;

    union {
        struct {
            char *name;

            union {
                /* eObrModule */
                struct {
                    vector_t *imports;
                    vector_t *decls;
                };

                /* eObrDeclGlobal */
                struct {
                    bool mut;
                    obr_t *type;
                    obr_t *value;
                };

                /* eObrImport|eObrTypeQual|eObrTypeName */
                char *symbol;
            };
        };
    };
} obr_t;

obr_t *obr_module(scan_t *scan, where_t where, char *name, vector_t *imports, vector_t *decls);
obr_t *obr_import(scan_t *scan, where_t where, char *name, char *symbol);

obr_t *obr_decl_global(scan_t *scan, where_t where, bool mut, char *name, obr_t *type, obr_t *value);

obr_t *obr_type_name(scan_t *scan, where_t where, char *symbol);
obr_t *obr_type_qual(scan_t *scan, where_t where, char *name, char *symbol);

/* special functions */

typedef struct obr_partial_value_t {
    scan_t *scan;
    where_t where;
    char *name;
} obr_partial_value_t;

obr_partial_value_t *obr_partial_value(scan_t *scan, where_t where, char *name);

vector_t *obr_expand_values(bool mut, vector_t *names, obr_t *type);
