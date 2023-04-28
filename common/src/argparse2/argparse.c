#include "argparse2/argparse.h"

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

static ap_param_t *ap_param_new(ap_param_type_t type, const char *desc, const char **names)
{
    ap_param_t *self = ctu_malloc(sizeof(ap_param_t));
    self->type = type;
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

/// public api

ap_t *ap_new(const char *desc, version_t version)
{
    ap_t *self = ctu_malloc(sizeof(ap_t));
    
    self->desc = desc;
    self->version = version;

    self->nameLookup = map_optimal(256);
    self->eventLookup = map_optimal(256);
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
    return self;
}

ap_param_t *ap_param_bool(
    const char *description, 
    const char **names
)
{
    ap_param_t *self = ap_param_new(eParamBool, description, names);
    return self;
}

ap_param_t *ap_param_int(
    const char *description, 
    const char **names
)
{
    ap_param_t *self = ap_param_new(eParamInt, description, names);
    return self;
}

ap_param_t *ap_param_string(
    const char *description, 
    const char **names
)
{
    ap_param_t *self = ap_param_new(eParamString, description, names);
    return self;
}

void ap_add(ap_group_t *self, ap_param_t *param)
{
    CTASSERT(self != NULL);
    CTASSERT(param != NULL);
    CTASSERT(*param->names != NULL);
    
    ap_t *parent = self->parent;
    const char *name = *param->names;

    while (name != NULL)
    {
        CTASSERT(map_get_ptr(self->parent->nameLookup, name) == NULL);

        map_set_ptr(parent->nameLookup, name, param);
    }

    map_set_ptr(parent->eventLookup, param, vector_new(4));
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

int ap_parse(ap_t *self, reports_t *reports, const char **argv, int argc)
{
    char *args = join_args(argc, argv);
    io_t *io = io_string("<command-line>", args);
    scan_t *scan = scan_io(reports, "ap2", io, self);

    scan_set(scan, self);
    compile_scanner(scan, &kCallbacks);

    return 0;
}
