// SPDX-License-Identifier: LGPL-3.0-only

#include "common.h"

#include "base/panic.h"
#include "arena/arena.h"

#include "std/map.h"
#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"

#include "config/config.h"

// internals

static ap_callback_t *ap_callback_new(ap_event_t event, void *data, arena_t *arena)
{
    ap_callback_t *self = ARENA_MALLOC(sizeof(ap_callback_t), "posarg_callback", NULL, arena);
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
    vector_t *events = map_get(self->event_lookup, param);

    if (events == NULL)
    {
        events = vector_init(cb, self->arena);
    }
    else
    {
        vector_push(&events, cb);
    }

    map_set(self->event_lookup, param, events);
}

/// public api

static void add_arg(ap_t *ap, const char *arg, cfg_field_t *field)
{
    const cfg_field_t *existing = map_get(ap->name_lookup, arg);
    if (existing != NULL)
    {
        const cfg_info_t *info = cfg_get_info(field);
        const cfg_info_t *prev = cfg_get_info(existing);
        CT_NEVER("a flag `%s` already exists (new: %s, old: %s)", arg, info->name, prev->name);
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

// construction

ap_t *ap_new(cfg_group_t *config, arena_t *arena)
{
    CTASSERT(config != NULL);

    ap_t *self = ARENA_MALLOC(sizeof(ap_t), "argparse", config, arena);

    self->arena = arena;
    self->config = config;

    self->name_lookup = map_optimal(256, kTypeInfoString, arena);
    self->event_lookup = map_optimal(256, kTypeInfoPtr, arena);

    self->posarg_callbacks = vector_new(16, arena);

    self->posargs = vector_new(16, arena);
    self->unknown = vector_new(16, arena);
    self->errors = vector_new(16, arena);
    self->count = 0;

    ARENA_IDENTIFY(self->name_lookup, "name_lookup", self, arena);
    ARENA_IDENTIFY(self->event_lookup, "event_lookup", self, arena);
    ARENA_IDENTIFY(self->posarg_callbacks, "posarg_callbacks", self, arena);
    ARENA_IDENTIFY(self->posargs, "posargs", self, arena);
    ARENA_IDENTIFY(self->unknown, "unknown", self, arena);
    ARENA_IDENTIFY(self->errors, "errors", self, arena);

    add_config_fields(self, config);

    return self;
}

void ap_update(ap_t *self)
{
    CTASSERT(self != NULL);

    // TODO: this is a bit aggressive
    map_reset(self->name_lookup);
    map_reset(self->event_lookup);

    add_config_fields(self, self->config);
}

void ap_event(ap_t *self, const cfg_field_t *param, ap_event_t callback, void *data)
{
    CTASSERT(self != NULL);
    CTASSERT(callback != NULL);

    ap_callback_t *fn = ap_callback_new(callback, data, self->arena);
    ARENA_REPARENT(fn, self, self->arena);

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
