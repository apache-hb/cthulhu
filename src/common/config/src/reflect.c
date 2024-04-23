// SPDX-License-Identifier: LGPL-3.0-only

#include "common.h"

STA_DECL
cfg_type_t cfg_get_type(const cfg_field_t *field)
{
    CTASSERT(field != NULL);

    return field->type;
}

STA_DECL
const cfg_info_t *cfg_get_info(const cfg_field_t *field)
{
    CTASSERT(field != NULL);

    return field->info;
}

STA_DECL
const cfg_info_t *cfg_group_info(const cfg_group_t *config)
{
    CTASSERT(config != NULL);

    return config->info;
}

STA_DECL
typevec_t *cfg_get_groups(const cfg_group_t *config)
{
    CTASSERT(config != NULL);

    return config->groups;
}

STA_DECL
vector_t *cfg_get_fields(const cfg_group_t *config)
{
    CTASSERT(config != NULL);

    return config->fields;
}

STA_DECL
const cfg_int_t *cfg_int_info(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigInt);

    return &field->int_config;
}

STA_DECL
bool cfg_bool_info(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigBool);

    return field->bool_config;
}

STA_DECL
const char *cfg_string_info(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigString);

    return field->string_config;
}

STA_DECL
const vector_t *cfg_vector_info(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigVector);

    return field->vec_config;
}

STA_DECL
const cfg_enum_t *cfg_enum_info(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigEnum);

    return &field->enum_config;
}

STA_DECL
const cfg_enum_t *cfg_flags_info(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigFlags);

    return &field->enum_config;
}

static const char *const kConfigTypeNames[eConfigCount] = {
#define CFG_TYPE(id, name) [id] = (name),
#include "config/config.inc"
};

STA_DECL
const char *cfg_type_string(cfg_type_t type)
{
    CT_ASSERT_RANGE(type, 0, eConfigCount - 1);

    return kConfigTypeNames[type];
}

static const char *const kConfigArgNames[eArgCount] = {
#define CFG_ARG(id, name, prefix) [id] = (name),
#include "config/config.inc"
};

STA_DECL
const char *cfg_arg_string(arg_style_t style)
{
    CT_ASSERT_RANGE(style, 0, eArgCount - 1);

    return kConfigArgNames[style];
}

static const char *const kConfigArgPrefixes[eArgCount] = {
#define CFG_ARG(id, name, prefix) [id] = (prefix),
#include "config/config.inc"
};

STA_DECL
const char *cfg_arg_prefix(arg_style_t style)
{
    CT_ASSERT_RANGE(style, 0, eArgCount - 1);

    return kConfigArgPrefixes[style];
}

/// access

STA_DECL
int cfg_int_value(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigInt);

    return field->int_value;
}

STA_DECL
bool cfg_bool_value(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigBool);

    return field->bool_value;
}

STA_DECL
const char *cfg_string_value(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigString);

    return field->string_value;
}

STA_DECL
vector_t *cfg_vector_value(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigVector);

    return field->vec_value;
}

STA_DECL
size_t cfg_enum_value(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigEnum);

    return field->enum_value;
}

STA_DECL
size_t cfg_flags_value(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigFlags);

    return field->flags_value;
}
