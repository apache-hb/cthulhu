#include "common.h"

#include "memory/arena.h"

#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"

// all this is put behind the same macro used for asserts
// because i dont trust compilers to optimize it out

#if CTU_PARANOID
static const cfg_field_t *config_find(const cfg_group_t *config, const char *name)
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
#endif

#define ASSERT_OPTIONS_VALID(options, count)                                                \
    CTASSERT((options) != NULL);                                                            \
    CTASSERT((count) > 0);                                                                  \
    CT_PARANOID(options_validate(options, count));

#define ASSERT_INFO_VALID_GROUP(info) \
    CTASSERT((info) != NULL);         \
    CTASSERT((info)->name != NULL);

#define ASSERT_INFO_VALID(info)    \
    ASSERT_INFO_VALID_GROUP(info); \
    CT_PARANOID(info_validate(info));

#define ASSERT_CONFIG_VALID(config, info)                                                         \
    CTASSERT((config) != NULL);                                                                   \
    ASSERT_INFO_VALID(info);                                                                      \
    CT_PARANOID_ASSERTF(config_find(config, (info)->name) == NULL, "duplicate config field `%s`", \
                        (info)->name);

static cfg_field_t *add_field(cfg_group_t *config, const cfg_info_t *info, cfg_type_t type)
{
    cfg_field_t *field = ARENA_MALLOC(sizeof(cfg_field_t), info->name, config, config->arena);

    field->type = type;
    field->info = info;

    vector_push(&config->fields, field);

    return field;
}

static void config_init(cfg_group_t *config, arena_t *arena, const cfg_info_t *info)
{
    CTASSERT(config != NULL);
    ASSERT_INFO_VALID_GROUP(info);

    config->arena = arena;
    config->info = info;

    config->groups = typevec_new(sizeof(cfg_group_t), 4, arena);
    config->fields = vector_new_arena(4, arena);

    ARENA_IDENTIFY(config->groups, "groups", config, arena);
    ARENA_IDENTIFY(config->fields, "fields", config, arena);
}

cfg_group_t *config_root(arena_t *arena, const cfg_info_t *info)
{
    cfg_group_t *config = ARENA_MALLOC(sizeof(cfg_group_t), "config", NULL, arena);
    config_init(config, arena, info);
    return config;
}

cfg_field_t *config_int(cfg_group_t *group, const cfg_info_t *info, cfg_int_t cfg)
{
    ASSERT_CONFIG_VALID(group, info);

    int min = cfg.min;
    int max = cfg.max;

    CTASSERTF(min <= max, "invalid range %d-%d", min, max);
    CTASSERTF(cfg.initial >= min && cfg.initial <= max, "initial value %d out of range %d-%d",
              cfg.initial, min, max);

    cfg_field_t *field = add_field(group, info, eConfigInt);
    field->int_config = cfg;
    field->int_value = cfg.initial;

    return field;
}

cfg_field_t *config_bool(cfg_group_t *group, const cfg_info_t *info, bool initial)
{
    ASSERT_CONFIG_VALID(group, info);

    cfg_field_t *field = add_field(group, info, eConfigBool);
    field->bool_config = initial;
    field->bool_value = initial;

    return field;
}

cfg_field_t *config_string(cfg_group_t *group, const cfg_info_t *info, const char *initial)
{
    ASSERT_CONFIG_VALID(group, info);

    cfg_field_t *field = add_field(group, info, eConfigString);
    field->string_config = initial;
    field->string_value = initial != NULL ? arena_strdup(initial, group->arena) : NULL;

    return field;
}

cfg_field_t *config_vector(cfg_group_t *group, const cfg_info_t *info, vector_t *initial)
{
    ASSERT_CONFIG_VALID(group, info);

    cfg_field_t *field = add_field(group, info, eConfigVector);
    field->vec_value = initial != NULL ? initial : vector_new_arena(4, group->arena);

    return field;
}

cfg_field_t *config_enum(cfg_group_t *group, const cfg_info_t *info, cfg_enum_t cfg)
{
    ASSERT_CONFIG_VALID(group, info);
    ASSERT_OPTIONS_VALID(cfg.options, cfg.count);

    cfg_field_t *field = add_field(group, info, eConfigEnum);
    field->enum_config = cfg;
    field->enum_value = cfg.initial;

    return field;
}

cfg_field_t *config_flags(cfg_group_t *group, const cfg_info_t *info, cfg_flags_t cfg)
{
    ASSERT_CONFIG_VALID(group, info);
    ASSERT_OPTIONS_VALID(cfg.options, cfg.count);

    cfg_field_t *field = add_field(group, info, eConfigFlags);
    field->flags_config = cfg;
    field->flags_value = cfg.initial;

    return field;
}

cfg_group_t *config_group(cfg_group_t *group, const cfg_info_t *info)
{
    CTASSERT(group != NULL);

    cfg_group_t config = {0};
    config_init(&config, group->arena, info);

    cfg_group_t *result = typevec_push(group->groups, &config);

    ARENA_REPARENT(result, group, group->arena);

    return result;
}
