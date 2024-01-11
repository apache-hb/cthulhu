#include "common.h"

#include "base/panic.h"
#include "memory/arena.h"

#include "std/map.h"
#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"

#include "config/config.h"

#include <stdio.h>
#include <string.h>

// internals

static ap_callback_t *ap_callback_new(ap_event_t event, void *data, arena_t *arena)
{
    ap_callback_t *self = ARENA_MALLOC(arena, sizeof(ap_callback_t), "posarg_callback", NULL);
    self->callback = event;
    self->data = data;
    return self;
}

static void add_pos_callback(ap_t *self, ap_callback_t *cb)
{
    vector_push(&self->posarg_callbacks, cb);
}

static void add_arg_callback(ap_t *self, const cfg_field_t *param, ap_callback_t *cb)
{
    vector_t *events = map_get_ptr(self->event_lookup, param);
    CTASSERT(events != NULL);

    vector_push(&events, cb);

    map_set_ptr(self->event_lookup, param, events);
}

/// public api

static void add_arg(ap_t *ap, const char *arg, cfg_field_t *field)
{
    const cfg_field_t *existing = map_get_ptr(ap->name_lookup, arg);
    if (existing != NULL)
    {
        const cfg_info_t *info = cfg_get_info(field);
        const cfg_info_t *prev = cfg_get_info(existing);
        NEVER("a flag `%s` already exists (new: %s, old: %s)", arg, info->name, prev->name);
    }

    map_set(ap->name_lookup, arg, field);
}

static void add_single_field(ap_t *ap, cfg_field_t *field)
{
    CTASSERT(ap != NULL);
    CTASSERT(field != NULL);

    const cfg_info_t *info = cfg_get_info(field);

    // TODO: take the union of all the args when adding them
    if (info->short_args)
    {
        for (size_t i = 0; info->short_args[i]; i++)
        {
            add_arg(ap, info->short_args[i], field);
        }
    }

    if (info->long_args)
    {
        for (size_t i = 0; info->long_args[i]; i++)
        {
            add_arg(ap, info->long_args[i], field);
        }
    }
}

static void add_config_fields(ap_t *ap, const cfg_group_t *config)
{
    vector_t *fields = cfg_get_fields(config);
    size_t field_count = vector_len(fields);
    for (size_t i = 0; i < field_count; i++)
    {
        cfg_field_t *field = vector_get(fields, i);
        add_single_field(ap, field);
    }

    typevec_t *groups = cfg_get_groups(config);
    size_t group_count = typevec_len(groups);
    for (size_t i = 0; i < group_count; i++)
    {
        const cfg_group_t *group = typevec_offset(groups, i);
        add_config_fields(ap, group);
    }
}

ap_t *ap_new(cfg_group_t *config, arena_t *arena)
{
    CTASSERT(config != NULL);

    ap_t *self = ARENA_MALLOC(arena, sizeof(ap_t), "argparse", config);

    self->arena = arena;

    self->name_lookup = map_optimal(256, arena);
    self->event_lookup = map_optimal(256, arena);

    self->posarg_callbacks = vector_new_arena(16, arena);

    self->posargs = vector_new_arena(16, arena);
    self->unknown = vector_new_arena(16, arena);
    self->errors = vector_new_arena(16, arena);
    self->count = 0;

    ARENA_IDENTIFY(arena, self->name_lookup, "name_lookup", self);
    ARENA_IDENTIFY(arena, self->event_lookup, "event_lookup", self);
    ARENA_IDENTIFY(arena, self->posarg_callbacks, "posarg_callbacks", self);
    ARENA_IDENTIFY(arena, self->posargs, "posargs", self);
    ARENA_IDENTIFY(arena, self->unknown, "unknown", self);
    ARENA_IDENTIFY(arena, self->errors, "errors", self);

    add_config_fields(self, config);

    return self;
}

void ap_event(ap_t *self, const cfg_field_t *param, ap_event_t callback, void *data)
{
    CTASSERT(self != NULL);
    CTASSERT(callback != NULL);

    ap_callback_t *fn = ap_callback_new(callback, data, self->arena);
    ARENA_REPARENT(self->arena, fn, self);

    if (param == NULL)
    {
        add_pos_callback(self, fn);
    }
    else
    {
        add_arg_callback(self, param, fn);
    }
}

USE_DECL
vector_t *ap_get_posargs(ap_t *self)
{
    CTASSERT(self != NULL);

    return self->posargs;
}

USE_DECL
vector_t *ap_get_unknown(ap_t *self)
{
    CTASSERT(self != NULL);

    return self->unknown;
}

USE_DECL
vector_t *ap_get_errors(ap_t *self)
{
    CTASSERT(self != NULL);

    return self->errors;
}

size_t ap_count_params(ap_t *self)
{
    CTASSERT(self != NULL);

    return self->count;
}
