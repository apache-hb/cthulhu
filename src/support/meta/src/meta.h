// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "core/compiler.h"
#include "core/text.h"

typedef struct typevec_t typevec_t;
typedef struct json_t json_t;
typedef struct arena_t arena_t;
typedef struct scan_t scan_t;
typedef struct logger_t logger_t;

typedef struct meta_info_t meta_info_t;
typedef struct meta_ast_t meta_ast_t;
typedef struct meta_field_t meta_field_t;

typedef enum meta_type_t
{
    eMetaMpz,
    eMetaAst,
    eMetaString,
    eMetaVec,
    eMetaEnum,

    eMetaUnknown
} meta_type_t;

typedef struct meta_info_t
{
    text_view_t prefix;

    typevec_t *diagnostics; // typevec_t<diagnostic_t>
    typevec_t *nodes;      // typevec_t<meta_ast_t>
} meta_info_t;

typedef struct meta_field_t
{
    text_view_t name;
    meta_type_t type;
} meta_field_t;

typedef struct meta_ast_t
{
    text_view_t name;

    typevec_t *fields; // typevec_t<meta_field_t>
} meta_ast_t;

CT_LOCAL meta_info_t *meta_info_parse(json_t *json, scan_t *scan, logger_t *logger, arena_t *arena);
