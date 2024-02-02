#pragma once

#include <ctu_json_api.h>

#include "core/text.h"
#include "core/where.h"

#include <gmp.h>
#include <stdbool.h>

CT_BEGIN_API

typedef struct vector_t vector_t;
typedef struct typevec_t typevec_t;
typedef struct map_t map_t;
typedef struct node_t node_t;
typedef struct scan_t scan_t;

typedef enum json_kind_t
{
#define JSON_TYPE(id, str) id,
#include "json/json.def"

    eJsonCount
} json_kind_t;

typedef struct json_ast_t
{
    json_kind_t kind;
    const node_t *node;

    union {
        text_t string;
        mpz_t integer;
        float real;
        bool boolean;

        vector_t *array;
        map_t *object;
    };
} json_ast_t;

typedef struct json_member_t
{
    text_t key;
    json_ast_t *value;
} json_member_t;

CT_JSON_API
const char *json_kind_name(json_kind_t kind);

json_member_t json_member(text_t key, json_ast_t *value);

json_ast_t *json_ast_string(scan_t *scan, where_t where, text_t string);
json_ast_t *json_ast_integer(scan_t *scan, where_t where, mpz_t integer);
json_ast_t *json_ast_float(scan_t *scan, where_t where, float real);
json_ast_t *json_ast_boolean(scan_t *scan, where_t where, bool boolean);
json_ast_t *json_ast_array(scan_t *scan, where_t where, vector_t *array);
json_ast_t *json_ast_object(scan_t *scan, where_t where, typevec_t *members);
json_ast_t *json_ast_empty_object(scan_t *scan, where_t where);
json_ast_t *json_ast_null(scan_t *scan, where_t where);

CT_END_API
