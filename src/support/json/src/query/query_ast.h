// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
#include <ctu_json_api.h>

#include "core/text.h"

#include <gmp.h>

typedef struct scan_t scan_t;
typedef struct node_t node_t;
typedef struct query_ast_t query_ast_t;

typedef enum query_ast_type_t
{
    eQueryObject, // object
    eQueryField, // field.name
    eQueryMap, // field["name"]
    eQueryIndex, // field[0]

    eQueryCount
} query_ast_type_t;

typedef struct query_ast_t
{
    query_ast_type_t kind;

    union {
        struct {
            query_ast_t *object;
            union {
                /* eQueryField, eQueryMap */
                text_t field;

                /* eQueryIndex */
                mpz_t index;
            };
        };

        /* eQueryObject */
        text_t name;
    };
} query_ast_t;

CT_BEGIN_API

query_ast_t *query_ast_object(scan_t *scan, text_t name);
query_ast_t *query_ast_field(scan_t *scan, query_ast_t *object, text_t field);
query_ast_t *query_ast_map(scan_t *scan, query_ast_t *object, text_t field);
query_ast_t *query_ast_index(scan_t *scan, query_ast_t *object, mpz_t index);
