#include "common.h"

#include "base/panic.h"
#include "memory/memory.h"


#include "std/map.h"
#include "std/vector.h"

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

static ap_err_callback_t *ap_error_new(ap_error_t event, void *data, arena_t *arena)
{
    ap_err_callback_t *self = ARENA_MALLOC(arena, sizeof(ap_err_callback_t), "error_callback",
                                           NULL);
    self->callback = event;
    self->data = data;
    return self;
}

static ap_param_t *ap_param_new(ap_param_type_t type, const char *name, const char *desc,
                                const char **names, arena_t *arena)
{
    ap_param_t *self = ARENA_MALLOC(arena, sizeof(ap_param_t), name, NULL);
    self->type = type;
    self->name = name;
    self->desc = desc;
    self->names = names;
    return self;
}

static void add_pos_callback(ap_t *self, ap_callback_t *cb)
{
    vector_push(&self->posarg_callbacks, cb);
}

static void add_arg_callback(ap_t *self, ap_param_t *param, ap_callback_t *cb)
{
    vector_t *events = map_get_ptr(self->event_lookup, param);
    CTASSERT(events != NULL);

    vector_push(&events, cb);

    map_set_ptr(self->event_lookup, param, events);
}

static ap_param_t *add_param(ap_group_t *self, ap_param_type_t type, const char *name,
                             const char *desc, const char **names, arena_t *arena)
{
    CTASSERT(self != NULL);
    CTASSERT(desc != NULL);
    CTASSERT(*names != NULL);

    ap_param_t *param = ap_param_new(type, name, desc, names, arena);
    ARENA_REPARENT(self->parent->arena, param, self);

    ap_t *parent = self->parent;
    size_t idx = 0;

    while (names[idx] != NULL)
    {
        const ap_param_t *old = map_get(self->parent->name_lookup, names[idx]);
        if (old != NULL)
        {
            // TODO: better logging
            printf("failed to add name (name=%s,param=%s)\n", desc, names[idx]);
            printf("name already exists (param=%s)\n", old->desc);
            continue;
        }

        map_set(parent->name_lookup, names[idx], param);

        idx += 1;
    }

    map_set_ptr(parent->event_lookup, param, vector_new(4));
    vector_push(&self->params, param);

    return param;
}

/// public api

ap_t *ap_new(const char *desc, version_t version, arena_t *arena)
{
    ap_t *self = ARENA_MALLOC(arena, sizeof(ap_t), "argparse", NULL);

    self->arena = arena;
    self->desc = desc;
    self->version = version;

    self->name_lookup = map_optimal(256);
    self->event_lookup = map_optimal(256);
    self->param_values = map_optimal(256);
    self->groups = vector_new(16);

    self->posarg_callbacks = vector_new(16);
    self->error_callbacks = vector_new(16);

    ARENA_IDENTIFY(arena, self->name_lookup, "name_lookup", self);
    ARENA_IDENTIFY(arena, self->event_lookup, "event_lookup", self);
    ARENA_IDENTIFY(arena, self->param_values, "param_values", self);
    ARENA_IDENTIFY(arena, self->groups, "groups", self);
    ARENA_IDENTIFY(arena, self->posarg_callbacks, "posarg_callbacks", self);
    ARENA_IDENTIFY(arena, self->error_callbacks, "error_callbacks", self);

    return self;
}

ap_group_t *ap_group_new(ap_t *parent, const char *name, const char *desc)
{
    ap_group_t *self = ARENA_MALLOC(parent->arena, sizeof(ap_group_t), name, parent);
    self->parent = parent;
    self->name = name;
    self->desc = desc;
    self->params = vector_new(16);
    ARENA_IDENTIFY(self->parent->arena, self->params, "params", self);

    vector_push(&parent->groups, self);
    return self;
}

ap_param_t *ap_add_bool(ap_group_t *self, const char *name, const char *desc, const char **names)
{
    return add_param(self, eParamBool, name, desc, names, self->parent->arena);
}

ap_param_t *ap_add_int(ap_group_t *self, const char *name, const char *desc, const char **names)
{
    return add_param(self, eParamInt, name, desc, names, self->parent->arena);
}

ap_param_t *ap_add_string(ap_group_t *self, const char *name, const char *desc, const char **names)
{
    return add_param(self, eParamString, name, desc, names, self->parent->arena);
}

bool ap_get_bool(ap_t *self, const ap_param_t *param, bool *value)
{
    CTASSERT(self != NULL);
    CTASSERT(param != NULL);
    CTASSERT(value != NULL);

    CTASSERT(param->type == eParamBool);

    bool *val = map_get_ptr(self->param_values, param);
    if (val != NULL)
    {
        *value = *val;
        return true;
    }

    return false;
}

bool ap_get_int(ap_t *self, const ap_param_t *param, mpz_t value)
{
    CTASSERT(self != NULL);
    CTASSERT(param != NULL);
    CTASSERT(value != NULL);

    CTASSERT(param->type == eParamInt);

    void *val = map_get_ptr(self->param_values, param);
    if (val != NULL)
    {
        memcpy(value, val, sizeof(mpz_t));
        return true;
    }

    return false;
}

bool ap_get_string(ap_t *self, const ap_param_t *param, const char **value)
{
    CTASSERT(self != NULL);
    CTASSERT(param != NULL);
    CTASSERT(value != NULL);

    CTASSERT(param->type == eParamString);

    const char *val = map_get_ptr(self->param_values, param);
    if (val != NULL)
    {
        *value = val;
        return true;
    }

    return false;
}

void ap_event(ap_t *self, ap_param_t *param, ap_event_t callback, void *data)
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

void ap_error(ap_t *self, ap_error_t callback, void *data)
{
    CTASSERT(self != NULL);
    CTASSERT(callback != NULL);

    ap_err_callback_t *fn = ap_error_new(callback, data, self->arena);
    ARENA_REPARENT(self->arena, fn, self);

    vector_push(&self->error_callbacks, fn);
}

USE_DECL
const vector_t *ap_get_groups(const ap_t *self)
{
    CTASSERT(self != NULL);

    return self->groups;
}

USE_DECL
const vector_t *ap_get_params(const ap_group_t *self)
{
    CTASSERT(self != NULL);

    return self->params;
}
