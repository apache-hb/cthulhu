// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_setup_api.h>

typedef struct arena_t arena_t;
typedef struct io_t io_t;

typedef struct cfg_group_t cfg_group_t;
typedef struct cfg_field_t cfg_field_t;

CT_BEGIN_API

typedef struct setup_t
{
    arena_t *arena;

    io_t *io;
} setup_t;

typedef struct setup_options_t
{
    // default config group
    cfg_group_t *general_group;

    // print help and quit
    cfg_field_t *print_help;

    // print version and quit
    cfg_field_t *print_version;

    // print help with usage and quit
    cfg_field_t *enable_usage;

    // print help with windows style options
    cfg_field_t *enable_windows_style;

    // enable colour output
    cfg_field_t *colour_output;

    // debug config group
    cfg_group_t *debug_group;

    // enable debug verbosity
    cfg_field_t *log_verbose;
} setup_options_t;

typedef struct setup_init_t
{
    void *empty;
} setup_init_t;

/// @brief initialise the runtime with default options
CT_SETUP_API void setup_global(void);

CT_SETUP_API arena_t *electric_fence_arena(void);

CT_SETUP_API setup_options_t setup_options(cfg_group_t *root);

CT_END_API
