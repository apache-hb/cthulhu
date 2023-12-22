#pragma once

#include "argparse/argparse.h"

#include "scan/node.h"

#include <gmp.h>

#define APLTYPE where_t

typedef struct vector_t vector_t;
typedef struct map_t map_t;

typedef enum ap_param_type_t {
    eParamBool,
    eParamString,
    eParamInt,
    eParamGroup,

    eParamTotal
} ap_param_type_t;

typedef struct ap_group_t {
    ap_t *parent;
    const char *name;
    const char *desc;

    vector_t *params;
} ap_group_t;

typedef struct ap_param_t {
    ap_param_type_t type;

    const char *name;
    const char *desc;

    const char *const *names;
} ap_param_t;

#define CALLBACK_TYPE(NAME, TYPE) \
    typedef struct NAME { \
        TYPE callback; \
        void *data; \
    } NAME;

CALLBACK_TYPE(ap_callback_t, ap_event_t)
CALLBACK_TYPE(ap_err_callback_t, ap_error_t)

typedef struct ap_t
{
    arena_t *arena;
    config_t *config;

    const char *desc;
    version_t version;

    // name -> ap_param_t lookup
    map_t *name_lookup;

    // param -> vector<ap_event_t> lookup
    map_t *event_lookup;

    // param -> void* lookup
    map_t *param_values;

    // all groups
    vector_t *groups;

    // vector<ap_callback_t> for positional arguments
    vector_t *posarg_callbacks;

    // vector<ap_err_callback_t> for error callbacks
    vector_t *error_callbacks;
} ap_t;

void ap_on_string(scan_t *scan, where_t where, const ap_param_t *param, const char *value);
void ap_on_bool(scan_t *scan, where_t where, const ap_param_t *param, bool value);
void ap_on_int(scan_t *scan, where_t where, const ap_param_t *param, mpz_t value);
void ap_on_posarg(scan_t *scan, where_t where, const char *value);

void ap_on_error(scan_t *scan, where_t where, const char *message);

int ap_get_opt(ap_t *self, const char *name, ap_param_t **param, char **error);
