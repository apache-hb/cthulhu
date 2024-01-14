#include "common.h"

#include "std/str.h"
#include "std/vector.h"

bool cfg_set_int(cfg_field_t *field, int value)
{
    ASSERT_FIELD_TYPE(field, eConfigInt);

    int min = field->int_config.min;
    int max = field->int_config.max;

    if (value < min || value > max)
    {
        return false;
    }

    field->int_value = value;

    return true;
}

void cfg_set_bool(cfg_field_t *field, bool value)
{
    ASSERT_FIELD_TYPE(field, eConfigBool);

    field->bool_value = value;
}

void cfg_set_string(cfg_field_t *field, char *value)
{
    ASSERT_FIELD_TYPE(field, eConfigString);

    field->string_value = value;
}

void cfg_vector_push(cfg_field_t *field, char *value)
{
    ASSERT_FIELD_TYPE(field, eConfigVector);

    vector_push(&field->vec_value, value);
}

bool cfg_set_enum(cfg_field_t *field, const char *choice)
{
    ASSERT_FIELD_TYPE(field, eConfigEnum);
    CTASSERT(choice != NULL);

    cfg_enum_t cfg = field->enum_config;

    for (size_t i = 0; i < cfg.count; i++)
    {
        cfg_choice_t option = cfg.options[i];
        if (str_equal(choice, option.text))
        {
            field->enum_value = option.value;
            return true;
        }
    }

    return false;
}

#if CTU_PARANOID
static bool is_valid_choice(const cfg_enum_t *options, size_t value)
{
    const cfg_choice_t *choices = options->options;
    size_t len = options->count;

    for (size_t i = 0; i < len; i++)
    {
        cfg_choice_t choice = choices[i];
        if (choice.value == value)
        {
            return true;
        }
    }

    return false;
}
#endif

void cfg_set_enum_value(cfg_field_t *field, size_t value)
{
    ASSERT_FIELD_TYPE(field, eConfigEnum);
    CT_PARANOID_ASSERTF(is_valid_choice(&field->enum_config, value), "invalid enum value %zu for field %s", value, field->info->name);

    field->enum_value = value;
}

static void set_flag(cfg_field_t *field, size_t value, bool set)
{
    if (set)
    {
        field->flags_value |= value;
    }
    else
    {
        field->flags_value &= ~value;
    }
}

bool cfg_set_flag(cfg_field_t *field, const char *choice, bool set)
{
    ASSERT_FIELD_TYPE(field, eConfigFlags);
    CTASSERT(choice != NULL);

    cfg_flags_t cfg = field->flags_config;

    for (size_t i = 0; i < cfg.count; i++)
    {
        cfg_choice_t option = cfg.options[i];
        if (str_equal(choice, option.text))
        {
            set_flag(field, option.value, set);
            return true;
        }
    }

    return false;
}

#if CTU_PARANOID
static bool is_valid_flag(const cfg_flags_t *options, size_t value)
{
    if (value == 0) return true;

    const cfg_choice_t *choices = options->options;
    size_t len = options->count;

    size_t remaining = value;
    for (size_t i = 0; i < len; i++)
    {
        cfg_choice_t choice = choices[i];
        if (choice.value != 0 && remaining & choice.value)
        {
            remaining &= ~choice.value;
        }
    }

    return remaining == 0;
}
#endif

void cfg_set_flag_value(cfg_field_t *field, size_t value)
{
    ASSERT_FIELD_TYPE(field, eConfigFlags);
    CT_PARANOID_ASSERTF(is_valid_flag(&field->flags_config, value), "invalid flag value %zu for field %s", value, field->info->name);

    field->flags_value = value;
}
