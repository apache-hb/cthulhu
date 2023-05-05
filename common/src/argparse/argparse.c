#include "argparse/argparse.h"

#include "common.h"

#include "base/memory.h"
#include "base/panic.h"

#include "std/map.h"
#include "std/vector.h"
#include "std/str.h"

#include "io/io.h"

#include "scan/compile.h"

#include "ap-bison.h"
#include "ap-flex.h"

// internals

CT_CALLBACKS(kCallbacks, ap);

static ap_callback_t *ap_callback_new(ap_event_t event, void *data)
{
    ap_callback_t *self = ctu_malloc(sizeof(ap_callback_t));
    self->callback = event;
    self->data = data;
    return self;
}

static ap_err_callback_t *ap_error_new(ap_error_t event, void *data)
{
    ap_err_callback_t *self = ctu_malloc(sizeof(ap_err_callback_t));
    self->callback = event;
    self->data = data;
    return self;
}

static ap_param_t *ap_param_new(ap_param_type_t type, const char *name, const char *desc, const char **names)
{
    ap_param_t *self = ctu_malloc(sizeof(ap_param_t));
    self->type = type;
    self->name = name;
    self->desc = desc;
    self->names = names;
    return self;
}

static void add_pos_callback(ap_t *self, ap_callback_t *cb)
{
    vector_push(&self->posArgCallbacks, cb);
}

static void add_arg_callback(ap_t *self, ap_param_t *param, ap_callback_t *cb)
{
    vector_t *events = map_get_ptr(self->eventLookup, param);
    CTASSERT(events != NULL);

    vector_push(&events, cb);

    map_set_ptr(self->eventLookup, param, events);
}

static char *join_args(int argc, const char **argv)
{
    vector_t *vec = vector_of(argc - 1);
    for (int i = 1; i < argc; i++)
    {
        vector_set(vec, i - 1, (char *)argv[i]);
    }
    return str_join(" ", vec);
}

static ap_param_t *add_param(ap_group_t *self, ap_param_type_t type, const char *name, const char *desc, const char **names)
{
    CTASSERT(self != NULL);
    CTASSERT(desc != NULL);
    CTASSERT(*names != NULL);
    
    ap_param_t *param = ap_param_new(type, name, desc, names);

    ap_t *parent = self->parent;
    size_t idx = 0;

    while (names[idx] != NULL)
    {
        const ap_param_t *old = map_get(self->parent->nameLookup, names[idx]);
        if (old != NULL)
        {
            printf("failed to add name (name=%s,param=%s)\n", desc, names[idx]);
            printf("name already exists (param=%s)\n", old->desc);
            continue;
        }

        map_set(parent->nameLookup, names[idx], param);

        idx += 1;
    }

    map_set_ptr(parent->eventLookup, param, vector_new(4));
    vector_push(&self->params, param);

    return param;
}


/// public api

ap_t *ap_new(const char *desc, version_t version)
{
    ap_t *self = ctu_malloc(sizeof(ap_t));
    
    self->desc = desc;
    self->version = version;

    self->nameLookup = map_optimal(256);
    self->eventLookup = map_optimal(256);
    self->paramValues = map_optimal(256);
    self->groups = vector_new(16);

    self->posArgCallbacks = vector_new(16);
    self->errorCallbacks = vector_new(16);

    return self;
}

ap_group_t *ap_group_new(
    ap_t *parent, 
    const char *name, 
    const char *desc
)
{
    ap_group_t *self = ctu_malloc(sizeof(ap_group_t));
    self->parent = parent;
    self->name = name;
    self->desc = desc;
    self->params = vector_new(16);

    vector_push(&parent->groups, self);
    return self;
}

ap_param_t *ap_add_bool(ap_group_t *self, const char *name, const char *desc, const char **names)
{
    return add_param(self, eParamBool, name, desc, names);
}

ap_param_t *ap_add_int(ap_group_t *self, const char *name, const char *desc, const char **names)
{
    return add_param(self, eParamInt, name, desc, names);
}

ap_param_t *ap_add_string(ap_group_t *self, const char *name, const char *desc, const char **names)
{
    return add_param(self, eParamString, name, desc, names);
}

bool ap_get_bool(ap_t *self, const ap_param_t *param, bool *value)
{
    CTASSERT(self != NULL);
    CTASSERT(param != NULL);
    CTASSERT(value != NULL);

    CTASSERT(param->type == eParamBool);

    bool *val = map_get_ptr(self->paramValues, param);
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

    void *val = map_get_ptr(self->paramValues, param);
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

    const char *val = map_get_ptr(self->paramValues, param);
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

    ap_callback_t *fn = ap_callback_new(callback, data);

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

    vector_push(&self->errorCallbacks, ap_error_new(callback, data));
}

int ap_parse(ap_t *self, reports_t *reports, int argc, const char **argv)
{
    char *args = join_args(argc, argv);
    io_t *io = io_string("<command-line>", args);
    scan_t *scan = scan_io(reports, "ap2", io, self);

    scan_set(scan, self);
    compile_scanner(scan, &kCallbacks);

    return 0;
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
