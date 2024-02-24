// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_json_api.h>

#include "json/json.h"

#include "core/where.h"

typedef struct scan_t scan_t;
typedef struct typevec_t typevec_t;

typedef struct json_member_t
{
    text_t key;
    json_t *value;
} json_member_t;

json_member_t json_member(text_t key, json_t *value);

json_t *json_ast_string(scan_t *scan, where_t where, text_t string);
json_t *json_ast_integer(scan_t *scan, where_t where, mpz_t integer);
json_t *json_ast_float(scan_t *scan, where_t where, float real);
json_t *json_ast_boolean(scan_t *scan, where_t where, bool boolean);
json_t *json_ast_array(scan_t *scan, where_t where, const vector_t *array);
json_t *json_ast_object(scan_t *scan, where_t where, const typevec_t *members);
json_t *json_ast_empty_object(scan_t *scan, where_t where);
json_t *json_ast_null(scan_t *scan, where_t where);
