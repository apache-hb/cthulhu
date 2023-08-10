#pragma once

#include "scan/scan.h"
#include "scan/node.h"

#include <gmp.h>

typedef struct ctu_t ctu_t;
typedef struct vector_t vector_t;

typedef enum ctu_kind_t {
    eCtuExprInt,
    eCtuExprBool,

    eCtuTypeName,
    eCtuTypePointer,

    eCtuDeclGlobal,
    eCtuDeclFunction,

    eCtuDeclStruct,
    eCtuField,

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
                struct {
                    ctu_t *type;
                    ctu_t *global;
                    bool mut;
                };

                /* eCtuDeclFunction */
                struct {
                    ctu_t *returnType;
                };

                /* eCtuDeclStruct */
                vector_t *fields;

                /* eCtuField */
                ctu_t *fieldType;
            };
        };

        /* eCtuExprInt */
        mpz_t intValue;

        /* eCtuExprBool */
        bool boolValue;

        /* eCtuTypeName */
        vector_t *typeName;

        /* eCtuTypePointer */
        ctu_t *pointer;

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

ctu_t *ctu_expr_int(scan_t *scan, where_t where, mpz_t value);
ctu_t *ctu_expr_bool(scan_t *scan, where_t where, bool value);

ctu_t *ctu_type_name(scan_t *scan, where_t where, vector_t *path);
ctu_t *ctu_type_pointer(scan_t *scan, where_t where, ctu_t *pointer);

ctu_t *ctu_decl_global(scan_t *scan, where_t where, bool exported, bool mutable, char *name, ctu_t *type, ctu_t *global);
ctu_t *ctu_decl_function(scan_t *scan, where_t where, bool exported, char *name, ctu_t *returnType);

ctu_t *ctu_decl_struct(scan_t *scan, where_t where, bool exported, char *name, vector_t *fields);
ctu_t *ctu_field(scan_t *scan, where_t where, char *name, ctu_t *type);
