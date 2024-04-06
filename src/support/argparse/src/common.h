// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "argparse/argparse.h"

#include "config/config.h"
#include "core/analyze.h"

#include <gmp.h>

#define APLTYPE where_t

typedef struct vector_t vector_t;
typedef struct map_t map_t;
typedef struct scan_t scan_t;

typedef struct ap_callback_t
{
    ap_event_t callback;
    void *data;
} ap_callback_t;

/// @brief argparse instance
typedef struct ap_t
{
    /// @brief allocation arena
    arena_t *arena;

    /// @brief the root config group
    cfg_group_t *config;

    /// @brief a mapping of names to parameters
    // map_t<const char*, cfg_field_t*>
    map_t *name_lookup;

    // param -> vector<ap_event_t> lookup
    map_t *event_lookup;

    // vector<ap_callback_t> for positional arguments
    vector_t *posarg_callbacks;

    /// @brief all positional arguments
    /// ie arguments without a leading dash or slash
    // vector_t<const char*>
    vector_t *posargs;

    /// @brief all unknown arguments
    /// arguments that the config did not register at startup
    // vector_t<const char*>
    vector_t *unknown;

    /// @brief all errors
    /// errors other than unknown arguments
    vector_t *errors;

    /// @brief tracks the number of encountered arguments
    /// only counts arguments that are not positional or unknown
    size_t count;
} ap_t;

typedef struct ap_field_t
{
    cfg_field_t *field;
    bool negate;
} ap_field_t;

CT_ARGPARSE_API int ap_parse_common(ap_t *self, const char *text);

CT_ARGPARSE_API void ap_on_string(scan_t *scan, cfg_field_t *param, char *value);
CT_ARGPARSE_API void ap_on_bool(scan_t *scan, cfg_field_t *param, bool value);
CT_ARGPARSE_API void ap_on_int(scan_t *scan, cfg_field_t *param, mpz_t value);

CT_ARGPARSE_API void ap_on_posarg(scan_t *scan, char *value);

CT_PRINTF(2, 3)
CT_ARGPARSE_API void ap_add_error(ap_t *self, const char *fmt, ...);

CT_ARGPARSE_API void ap_on_invalid(scan_t *scan, char *value);

CT_ARGPARSE_API int ap_get_opt(scan_t *scan, const char *name, ap_field_t *param, char **value);
