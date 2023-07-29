#pragma once

#include "scan/scan.h"
#include "scan/node.h"

typedef struct ctu_t ctu_t;
typedef struct vector_t vector_t;

typedef enum ctu_kind_t {
    eCtuGlobal,

    eCtuImport,
    eCtuModule
} ctu_kind_t;

typedef struct ctu_t {
    ctu_kind_t kind;
    node_t *node;

    union {
        struct {
            vector_t *importPath;
            char *alias;
        };

        struct {
            vector_t *modspec;
            vector_t *imports;
            vector_t *decls;
        };

        struct {
            char *name;
            bool exported;

            bool mut;
        };
    };
} ctu_t;

ctu_t *ctu_module(scan_t *scan, where_t where, vector_t *modspec, vector_t *imports, vector_t *decls);
ctu_t *ctu_import(scan_t *scan, where_t where, vector_t *path, char *alias);

ctu_t *ctu_global(scan_t *scan, where_t where, bool exported, bool mutable, char *name);
