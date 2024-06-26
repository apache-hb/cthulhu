// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "config/config.h"

#include "base/panic.h"

typedef struct typevec_t typevec_t;

typedef struct cfg_field_t
{
    cfg_type_t type;
    const cfg_info_t *info;

    union {
        cfg_int_t int_config;
        bool bool_config;
        const char *string_config;
        vector_t *vec_config;
        cfg_enum_t enum_config;
    };

    union {
        int int_value;
        bool bool_value;
        char *string_value;
        size_t enum_value;
        size_t flags_value;
        vector_t *vec_value;
    };
} cfg_field_t;

typedef struct cfg_group_t
{
    arena_t *arena;
    const cfg_info_t *info;

    typevec_t *groups;
    vector_t *fields;
} cfg_group_t;

#define ASSERT_FIELD_TYPE(field, type) CTASSERT(cfg_get_type(field) == (type))
