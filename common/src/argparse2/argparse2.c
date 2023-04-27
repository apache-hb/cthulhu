#include "argparse2/argparse2.h"

#include "base/memory.h"
#include "base/panic.h"

#include "std/map.h"
#include "std/vector.h"

typedef enum ap2_param_type_t
{
    eParamBool,
    eParamString,
    eParamInt,

    eParamTotal
} ap2_param_type_t;

typedef struct ap2_group_t
{
    ap2_t *parent;
    const char *name;
    const char *desc;
} ap2_group_t;

typedef struct ap2_param_t
{
    ap2_param_type_t type;
    const char *desc;
    const char **names;

    union {
        bool defaultBoolValue;
        const char *defaultStringValue;
        int defaultIntValue;
    };
} ap2_param_t;

typedef struct ap2_callback_t
{
    ap2_event_t event;
    void *data;
} ap2_callback_t;

typedef struct ap2_t
{
    // name -> ap2_param_t lookup
    map_t *nameLookup;

    // param -> vector<ap2_event_t> lookup
    map_t *eventLookup;

    // all groups
    vector_t *groups;

    // vector<ap2_callback_t> for positional arguments
    vector_t *posArgCallbacks;
} ap2_t;

// internals

static ap2_callback_t *ap2_callback_new(ap2_event_t event, void *data)
{
    ap2_callback_t *self = ctu_malloc(sizeof(ap2_callback_t));
    self->event = event;
    self->data = data;
    return self;
}

static ap2_param_t *ap2_param_new(ap2_param_type_t type, const char *desc, const char **names)
{
    ap2_param_t *self = ctu_malloc(sizeof(ap2_param_t));
    self->type = type;
    self->desc = desc;
    self->names = names;
    return self;
}

static void add_pos_callback(ap2_t *self, ap2_callback_t *cb)
{
    vector_push(&self->posArgCallbacks, cb);
}

static void add_arg_callback(ap2_t *self, ap2_param_t *param, ap2_callback_t *cb)
{
    vector_t *events = map_get_ptr(self->eventLookup, param);
    CTASSERT(events != NULL);

    vector_push(&events, cb);

    map_set_ptr(self->eventLookup, param, events);
}

/// public api

ap2_t *ap2_new(void)
{
    ap2_t *self = ctu_malloc(sizeof(ap2_t));
    self->nameLookup = map_optimal(256);
    self->eventLookup = map_optimal(256);
    self->groups = vector_new(16);
    return self;
}

ap2_group_t *ap2_group_new(
    ap2_t *parent, 
    const char *name, 
    const char *desc
)
{
    ap2_group_t *self = ctu_malloc(sizeof(ap2_group_t));
    self->parent = parent;
    self->name = name;
    self->desc = desc;
    return self;
}

ap2_param_t *ap2_param_bool(
    const char *description, 
    const char **names,
    bool defaultValue
)
{
    ap2_param_t *self = ap2_param_new(eParamBool, description, names);
    self->defaultBoolValue = defaultValue;
    return self;
}

ap2_param_t *ap2_param_int(
    const char *description, 
    const char **names,
    int defaultValue
)
{
    ap2_param_t *self = ap2_param_new(eParamInt, description, names);
    self->defaultIntValue = defaultValue;
    return self;
}

ap2_param_t *ap2_param_string(
    const char *description, 
    const char **names,
    const char *defaultValue
)
{
    ap2_param_t *self = ap2_param_new(eParamString, description, names);
    self->defaultStringValue = defaultValue;
    return self;
}

void ap2_add(ap2_group_t *self, ap2_param_t *param)
{
    CTASSERT(self != NULL);
    CTASSERT(param != NULL);
    CTASSERT(*param->names != NULL);
    
    ap2_t *parent = self->parent;
    const char *name = *param->names;

    while (name != NULL)
    {
        CTASSERT(map_get_ptr(self->parent->nameLookup, name) == NULL);

        map_set_ptr(parent->nameLookup, name, param);
    }

    map_set_ptr(parent->eventLookup, param, vector_new(4));
}

void ap2_event(ap2_t *self, ap2_param_t *param, ap2_event_t callback, void *data)
{
    CTASSERT(self != NULL);
    CTASSERT(callback != NULL);

    ap2_callback_t *fn = ap2_callback_new(callback, data);

    if (param == NULL)
    {
        add_pos_callback(self, fn);
    }
    else
    {
        add_arg_callback(self, param, fn);
    }
}
