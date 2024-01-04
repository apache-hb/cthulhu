#include "common.h"

cfg_type_t cfg_get_type(const cfg_field_t *field)
{
    CTASSERT(field != NULL);

    return field->type;
}

const cfg_info_t *cfg_get_info(const cfg_field_t *field)
{
    CTASSERT(field != NULL);

    return field->info;
}

const cfg_info_t *cfg_group_info(const cfg_group_t *config)
{
    CTASSERT(config != NULL);

    return config->info;
}

typevec_t *cfg_get_groups(const cfg_group_t *config)
{
    CTASSERT(config != NULL);

    return config->groups;
}

vector_t *cfg_get_fields(const cfg_group_t *config)
{
    CTASSERT(config != NULL);

    return config->fields;
}

const cfg_int_t *cfg_int_info(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigInt);

    return &field->int_config;
}

bool cfg_bool_info(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigBool);

    return field->bool_config;
}

const char *cfg_string_info(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigString);

    return field->string_config;
}

const cfg_enum_t *cfg_enum_info(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigEnum);

    return &field->enum_config;
}

const cfg_flags_t *cfg_flags_info(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigFlags);

    return &field->flags_config;
}

static const char *const kConfigTypeNames[eConfigTotal] = {
    [eConfigInt] = "int",
    [eConfigBool] = "bool",
    [eConfigString] = "string",
    [eConfigEnum] = "enum",
    [eConfigFlags] = "flags",
};

const char *cfg_type_name(cfg_type_t type)
{
    CTASSERTF(type < eConfigTotal, "invalid type %d", type);

    return kConfigTypeNames[type];
}

/// access

int cfg_int_value(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigInt);

    return field->int_value;
}

bool cfg_bool_value(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigBool);

    return field->bool_value;
}

const char *cfg_string_value(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigString);

    return field->string_value;
}

vector_t *cfg_vector_value(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigVector);

    return field->vec_value;
}

size_t cfg_enum_value(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigEnum);

    return field->enum_value;
}

size_t cfg_flags_value(const cfg_field_t *field)
{
    ASSERT_FIELD_TYPE(field, eConfigFlags);

    return field->flags_value;
}
