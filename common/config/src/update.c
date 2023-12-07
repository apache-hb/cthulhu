#include "common.h"

#include "memory/memory.h"

#include "std/str.h"

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

bool cfg_set_bool(cfg_field_t *field, bool value)
{
    ASSERT_FIELD_TYPE(field, eConfigBool);

    field->bool_value = value;

    return true;
}

bool cfg_set_string(cfg_field_t *field, const char *value)
{
    ASSERT_FIELD_TYPE(field, eConfigString);

    field->string_value = ctu_strdup(value);

    return true;
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

bool cfg_set_flag(cfg_field_t *field, const char *choice, bool value)
{
    ASSERT_FIELD_TYPE(field, eConfigFlags);
    CTASSERT(choice != NULL);

    cfg_flags_t cfg = field->flags_config;

    for (size_t i = 0; i < cfg.count; i++)
    {
        cfg_choice_t option = cfg.options[i];
        if (str_equal(choice, option.text))
        {
            if (value)
            {
                field->flags_value |= option.value;
            }
            else
            {
                field->flags_value &= ~option.value;
            }

            return true;
        }
    }

    return false;
}
