#pragma once

#include "scan/scan.h"
#include "scan/node.h"

#include <gmp.h>

typedef struct ctu_t ctu_t;
typedef struct vector_t vector_t;

typedef enum ctu_kind_t {
    eCtuExprInt,
    eCtuExprBool,

    eCtuStmtList,
    eCtuStmtLocal,

    eCtuTypeName,
    eCtuTypePointer,

    eCtuDeclGlobal,
    eCtuDeclFunction,

    eCtuDeclTypeAlias,
    eCtuDeclUnion,
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

                /* eCtuGlobal|eCtuStmtLocal */
                struct {
                    ctu_t *type;
                    ctu_t *value;
                    bool mut;
                };

                /* eCtuDeclFunction */
                struct {
                    ctu_t *returnType;
                    ctu_t *body;
                };

                /* eCtuDeclTypeAlias */
                struct {
                    bool newtype;
                    ctu_t *typeAlias;
                };

                /* eCtuDeclStruct|eCtuDeclUnion */
                vector_t *fields;

                /* eCtuField */
                ctu_t *fieldType;
            };
        };

        /* eCtuStmtList */
        vector_t *stmts;

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

///
/// modules
///

ctu_t *ctu_module(scan_t *scan, where_t where, vector_t *modspec, vector_t *imports, vector_t *decls);
ctu_t *ctu_import(scan_t *scan, where_t where, vector_t *path, char *name);

///
/// statements
///

ctu_t *ctu_stmt_list(scan_t *scan, where_t where, vector_t *stmts);
ctu_t *ctu_stmt_local(scan_t *scan, where_t where, bool mutable, char *name, ctu_t *type, ctu_t *value);

///
/// expressions
///

ctu_t *ctu_expr_int(scan_t *scan, where_t where, mpz_t value);
ctu_t *ctu_expr_bool(scan_t *scan, where_t where, bool value);

///
/// types
///

ctu_t *ctu_type_name(scan_t *scan, where_t where, vector_t *path);
ctu_t *ctu_type_pointer(scan_t *scan, where_t where, ctu_t *pointer);

///
/// real declarations
///

ctu_t *ctu_decl_global(scan_t *scan, where_t where, bool exported, bool mutable, char *name, ctu_t *type, ctu_t *value);
ctu_t *ctu_decl_function(scan_t *scan, where_t where, bool exported, char *name, ctu_t *returnType, ctu_t *body);

///
/// type declarations
///

ctu_t *ctu_decl_typealias(scan_t *scan, where_t where, bool exported, char *name, bool newtype, ctu_t *type);

ctu_t *ctu_decl_union(scan_t *scan, where_t where, bool exported, char *name, vector_t *fields);
ctu_t *ctu_decl_struct(scan_t *scan, where_t where, bool exported, char *name, vector_t *fields);

///
/// internal components
///

ctu_t *ctu_field(scan_t *scan, where_t where, char *name, ctu_t *type);
