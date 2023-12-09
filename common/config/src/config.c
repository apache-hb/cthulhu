#include "common.h"

#include "memory/memory.h"

#include "std/str.h"

#define ASSERT_INFO_VALID_GROUP(info) \
    CTASSERT(info != NULL);                                                                                               \
    CTASSERT(info->name != NULL); \

#define ASSERT_INFO_VALID(info)                                                                                        \
    ASSERT_INFO_VALID_GROUP(info);                                                                                               \
    CTASSERTF(info->arg_short != NULL || info->arg_long != NULL, "config `%s` must have a short or long argument", info->name);

static const cfg_field_t *config_find(const config_t *config, const char *name)
{
    size_t field_len = vector_len(config->fields);
    for (size_t i = 0; i < field_len; i++)
    {
        cfg_field_t *field = vector_get(config->fields, i);

        if (str_equal(field->info->name, name))
        {
            return field;
        }
    }

    return NULL;
}

static void options_validate(const cfg_choice_t *options, size_t count)
{
    CTASSERT(options != NULL);
    CTASSERT(count > 0);

    for (size_t i = 0; i < count; i++)
    {
        CTASSERT(options[i].text != NULL);
    }
}

#define ASSERT_CONFIG_VALID(config, info)                                                                         \
    CTASSERT(config != NULL);                                                                                          \
    ASSERT_INFO_VALID(info);                                                                                           \
    CTASSERTF(config_find(config, info->name) == NULL, "duplicate config field `%s`", info->name);

static cfg_field_t *add_field(config_t *config, const cfg_info_t *info, cfg_type_t type)
{
    cfg_field_t *field = ARENA_MALLOC(config->alloc, sizeof(cfg_field_t), info->name, config);
    field->info = info;
    field->type = type;

    vector_push(&config->fields, field);

    return field;
}

static config_t *alloc_config(arena_t *alloc, const cfg_info_t *info, const char *name)
{
    CTASSERT(alloc != NULL);
    ASSERT_INFO_VALID_GROUP(info);

    config_t *config = ARENA_MALLOC(alloc, sizeof(config_t), name, NULL);
    config->alloc = alloc;
    config->info = info;

    config->groups = vector_new(4);
    config->fields = vector_new(4);

    ARENA_IDENTIFY(alloc, config->groups, "groups", config);
    ARENA_IDENTIFY(alloc, config->fields, "fields", config);

    return config;
}

config_t *config_new(arena_t *alloc, const cfg_info_t *info)
{
    return alloc_config(alloc, info, "config");
}

cfg_field_t *config_int(config_t *group, const cfg_info_t *info, cfg_int_t cfg)
{
    ASSERT_CONFIG_VALID(group, info);

    int min = cfg.min;
    int max = cfg.max;

    CTASSERTF(min <= max, "invalid range %d-%d", min, max);
    CTASSERTF(cfg.initial >= min && cfg.initial <= max, "initial value %d out of range %d-%d", cfg.initial, min, max);

    cfg_field_t *field = add_field(group, info, eConfigInt);
    field->int_config = cfg;
    field->int_value = cfg.initial;

    return field;
}

cfg_field_t *config_bool(config_t *group, const cfg_info_t *info, cfg_bool_t cfg)
{
    ASSERT_CONFIG_VALID(group, info);

    cfg_field_t *field = add_field(group, info, eConfigBool);
    field->bool_config = cfg;
    field->bool_value = cfg.initial;

    return field;
}

cfg_field_t *config_string(config_t *group, const cfg_info_t *info, cfg_string_t cfg)
{
    ASSERT_CONFIG_VALID(group, info);

    cfg_field_t *field = add_field(group, info, eConfigString);
    field->string_config = cfg;
    field->string_value = ctu_strdup(cfg.initial);

    return field;
}

cfg_field_t *config_enum(config_t *group, const cfg_info_t *info, cfg_enum_t cfg)
{
    ASSERT_CONFIG_VALID(group, info);
    options_validate(cfg.options, cfg.count);

    cfg_field_t *field = add_field(group, info, eConfigEnum);
    field->enum_config = cfg;
    field->enum_value = cfg.initial;

    return NULL;
}

cfg_field_t *config_flags(config_t *group, const cfg_info_t *info, cfg_flags_t cfg)
{
    ASSERT_CONFIG_VALID(group, info);
    options_validate(cfg.options, cfg.count);

    cfg_field_t *field = add_field(group, info, eConfigFlags);
    field->flags_config = cfg;
    field->flags_value = cfg.initial;

    return NULL;
}

config_t *config_group(config_t *group, const cfg_info_t *info)
{
    CTASSERT(group != NULL);

    config_t *config = alloc_config(group->alloc, info, info->name);
    ARENA_REPARENT(group->alloc, config, group);
    vector_push(&group->groups, config);

    return config;
}
