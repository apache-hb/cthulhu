// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <ctu_schema_api.h>

CT_BEGIN_API

typedef struct schema_field_info_t schema_field_info_t;
typedef struct schema_type_t schema_type_t;
typedef struct schema_layout_t schema_layout_t;

typedef enum schema_status_t {
#define SCHEMA_STATUS(id, name) id,
#include "schema/schema.inc"

    eSchemaStatusCount
} schema_status_t;

typedef enum schema_typeof_t {
#define SCHEMA_TYPE(id, name) id,
#include "schema/schema.inc"

    eSchemaTypeCount
} schema_typeof_t;

typedef struct schema_type_t {
    schema_typeof_t type;
    const char *name;

    size_t size;
    size_t align;

    union {
        /* eSchemaTypePointer */
        const schema_type_t *pointer;
    };
} schema_type_t;

CT_END_API
