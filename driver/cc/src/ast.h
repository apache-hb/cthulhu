#pragma once

#include "std/vector.h"

#include "scan/node.h"

#include "cthulhu/tree/digit.h"

#include <gmp.h>

typedef enum cc_kind_t {
    eAstPath,

    eAstBool,
    eAstDigit,
    eAstPointer,

    eAstTypeDefine,

    eAstModule,
    eAstImport,

    eAstTotal
} cc_kind_t;

typedef struct cc_t {
    cc_kind_t kind;
    node_t *node;

    union {
        /* eAstPath */
        vector_t *path;

        /* eAstBool */
        /* empty */

        /* eAstDigit */
        struct
        {
            digit_t digit;
            sign_t sign;
        };

        /* eAstPointer */
        struct cc_t *ptr;

        /* any sort of declaration */
        struct {
            const char *name;

            union {
                /* eAstImport */
                vector_t *import;

                /* eAstTypeDef */
                struct cc_t *type;

                /* eAstModule */
                struct
                {
                    vector_t *modspec;
                    vector_t *imports;
                    vector_t *decls;
                };
            };
        };
    };
} cc_t;

cc_t *cc_module(scan_t *scan, where_t where, vector_t *path, vector_t *imports, vector_t *decls);
cc_t *cc_import(scan_t *scan, where_t where, vector_t *path, char *name);

cc_t *cc_typedef(scan_t *scan, where_t where, char *name, cc_t *type);

cc_t *cc_bool(scan_t *scan, where_t where);
cc_t *cc_digit(scan_t *scan, where_t where, sign_t sign, digit_t digit);
cc_t *cc_pointer(scan_t *scan, where_t where, cc_t *type);

cc_t *cc_path(scan_t *scan, where_t where, vector_t *path);
