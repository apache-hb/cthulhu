#include "common.h"

#include "memory/memory.h"

#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"

#include <limits.h>

// all this is put behind the same macro used for asserts
// because i dont trust compilers to optimize it out

#if CTU_DEBUG
static const cfg_field_t *config_find(const config_t *config, const char *name)
{
    size_t field_len = vector_len(config->fields);
    for (size_t i = 0; i < field_len; i++)
    {
        const cfg_field_t *field = vector_get(config->fields, i);

        if (str_equal(field->info->name, name))
        {
            return field;
        }
    }

    return NULL;
}

static void options_validate(const cfg_choice_t *options, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        CTASSERT(options[i].text != NULL);
    }
}

static bool has_any_element(const char *const *list)
{
    return list != NULL && *list != NULL;
}

static void info_validate(const cfg_info_t *info)
{
    CTASSERT(info != NULL);
    CTASSERT(info->name != NULL);
    CTASSERT(has_any_element(info->short_args) || has_any_element(info->long_args));
}

#define ASSERT_OPTIONS_VALID(options, count) \
    CTASSERT((options) != NULL); \
    CTASSERT((count) > 0); \
    options_validate(options, count)

#define ASSERT_INFO_VALID_GROUP(info) \
    CTASSERT((info) != NULL);           \
    CTASSERT((info)->name != NULL);

#define ASSERT_INFO_VALID(info)    \
    ASSERT_INFO_VALID_GROUP(info); \
    info_validate(info);

#define ASSERT_CONFIG_VALID(config, info)                                               \
    CTASSERT((config) != NULL);                                                         \
    ASSERT_INFO_VALID(info);                                                            \
    CTASSERTF(config_find(config, (info)->name) == NULL, "duplicate config field `%s`", \
              (info)->name);
#else
#   define ASSERT_OPTIONS_VALID(options, count)
#   define ASSERT_INFO_VALID_GROUP(info)
#   define ASSERT_INFO_VALID(info)
#   define ASSERT_CONFIG_VALID(config, info)
#endif

static cfg_field_t *add_field(config_t *config, const cfg_info_t *info, cfg_type_t type)
{
    cfg_field_t *field = ARENA_MALLOC(config->arena, sizeof(cfg_field_t), info->name, config);

    field->type = type;
    field->info = info;

    vector_push(&config->fields, field);

    return field;
}

static void config_init(config_t *config, arena_t *arena, const cfg_info_t *info)
{
    CTASSERT(config != NULL);
    ASSERT_INFO_VALID_GROUP(info);

    config->arena = arena;
    config->info = info;

    config->groups = typevec_new(sizeof(config_t), 4, arena);
    config->fields = vector_new_arena(4, arena);

    ARENA_IDENTIFY(arena, config->groups, "groups", config);
    ARENA_IDENTIFY(arena, config->fields, "fields", config);
}

config_t *config_new(arena_t *arena, const cfg_info_t *info)
{
    config_t *config = ARENA_MALLOC(arena, sizeof(config_t), "config", NULL);
    config_init(config, arena, info);
    return config;
}

cfg_field_t *config_int(config_t *group, const cfg_info_t *info, cfg_int_t cfg)
{
    ASSERT_CONFIG_VALID(group, info);

    int min = cfg.min;
    int max = cfg.max;

    if (min == 0 && max == 0)
    {
        cfg.min = INT_MIN;
        cfg.max = INT_MAX;
    }
    else
    {
        CTASSERTF(min <= max, "invalid range %d-%d", min, max);
        CTASSERTF(cfg.initial >= min && cfg.initial <= max, "initial value %d out of range %d-%d",
                cfg.initial, min, max);
    }

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
    field->string_value = cfg.initial != NULL ? ctu_strdup(cfg.initial) : NULL;

    return field;
}

cfg_field_t *config_enum(config_t *group, const cfg_info_t *info, cfg_enum_t cfg)
{
    ASSERT_CONFIG_VALID(group, info);
    ASSERT_OPTIONS_VALID(cfg.options, cfg.count);

    cfg_field_t *field = add_field(group, info, eConfigEnum);
    field->enum_config = cfg;
    field->enum_value = cfg.initial;

    return field;
}

cfg_field_t *config_flags(config_t *group, const cfg_info_t *info, cfg_flags_t cfg)
{
    ASSERT_CONFIG_VALID(group, info);
    ASSERT_OPTIONS_VALID(cfg.options, cfg.count);

    cfg_field_t *field = add_field(group, info, eConfigFlags);
    field->flags_config = cfg;
    field->flags_value = cfg.initial;

    return field;
}

config_t *config_group(config_t *group, const cfg_info_t *info)
{
    CTASSERT(group != NULL);

    config_t config = {0};
    config_init(&config, group->arena, info);

    config_t *result = typevec_push(group->groups, &config);

    ARENA_REPARENT(group->arena, result, group);

    return result;
}