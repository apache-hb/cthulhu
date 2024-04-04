// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_json_api.h>

#include "json/json.h"

#include "core/where.h"

typedef struct scan_t scan_t;
typedef struct typevec_t typevec_t;

typedef struct json_member_t
{
    text_view_t key;
    json_t value;
} json_member_t;

CT_LOCAL json_member_t json_member(text_view_t key, json_t value);

CT_LOCAL json_t json_ast_string(where_t where, text_view_t string);
CT_LOCAL json_t json_ast_integer(where_t where, mpz_t integer);
CT_LOCAL json_t json_ast_float(where_t where, float real);
CT_LOCAL json_t json_ast_boolean(where_t where, bool boolean);
CT_LOCAL json_t json_ast_array(where_t where, typevec_t array);
CT_LOCAL json_t json_ast_object(scan_t *scan, where_t where, const typevec_t *members);
CT_LOCAL json_t json_ast_empty_object(where_t where);
CT_LOCAL json_t json_ast_null(where_t where);
