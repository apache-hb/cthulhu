#pragma once

#include "scan/scan.h"
#include "scan/node.h"

typedef struct ctu_t ctu_t;
typedef struct vector_t vector_t;

typedef enum ctu_kind_t {
    eCtuTypeName,
    eCtuTypePointer,

    eCtuDeclGlobal,
    eCtuDeclFunction,

    eCtuImport,
    eCtuModule
} ctu_kind_t;

typedef struct ctu_t {
    ctu_kind_t kind;
    node_t *node;

    union {
        struct {
            char *name;
            bool exported;

            union {
                /* eCtuImport */
                vector_t *importPath;

                /* eCtuGlobal */
                bool mut;

                /* eCtuDeclFunction */
                ctu_t *returnType;
            };
        };

        /* eCtuTypeName */
        vector_t *typeName;

        /* eCtuTypePointer */
        ctu_t *type;

        /* eCtuModule */
        struct {
            vector_t *modspec;
            vector_t *imports;
            vector_t *decls;
        };
    };
} ctu_t;

ctu_t *ctu_module(scan_t *scan, where_t where, vector_t *modspec, vector_t *imports, vector_t *decls);
ctu_t *ctu_import(scan_t *scan, where_t where, vector_t *path, char *name);

ctu_t *ctu_type_name(scan_t *scan, where_t where, vector_t *path);
ctu_t *ctu_type_pointer(scan_t *scan, where_t where, ctu_t *type);

ctu_t *ctu_decl_global(scan_t *scan, where_t where, bool exported, bool mutable, char *name);
ctu_t *ctu_decl_function(scan_t *scan, where_t where, bool exported, char *name, ctu_t *returnType);
