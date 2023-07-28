#pragma once

#include "scan/node.h"

#include "std/vector.h"

#include <gmp.h>

typedef struct obr_t obr_t;

typedef enum obr_kind_t {
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
                vector_t *imports;

                /* eObrImport */
                char *symbol;
            };
        };
    };
} obr_t;

obr_t *obr_module(scan_t *scan, where_t where, char *name, vector_t *imports);
obr_t *obr_import(scan_t *scan, where_t where, char *name, char *symbol);
