#pragma once

#include "core/compiler.h"
#include "core/version_def.h"

BEGIN_API

typedef struct config_t config_t;
typedef struct cfg_field_t cfg_field_t;
typedef struct io_t io_t;
typedef struct arena_t arena_t;

typedef struct tool_config_t
{
    arena_t *arena;
    io_t *io;

    config_t *group;

    version_info_t version;

    int argc;
    const char **argv;
} tool_config_t;

typedef struct default_options_t
{
    // default config group
    config_t *group;

    // print help and quit
    cfg_field_t *print_help;

    // print version and quit
    cfg_field_t *print_version;

    // print help with usage and quit
    cfg_field_t *enable_usage;

    // print help with windows style options and quit
    cfg_field_t *enable_windows_style;

    // enable colour output
    cfg_field_t *colour_output;
} default_options_t;

/// @brief get the default options
///
/// @param group the config group to use
///
/// @return the default options
default_options_t get_default_options(config_t *group);

/// @brief process the default options
/// @note if this function does not return @a EXIT_OK, the program should exit
///       with the returned error code
///
/// @param options the default options
/// @param config the tool config
///
/// @return @a EXIT_OK on success or an error code
int process_default_options(default_options_t options, tool_config_t config);

int parse_commands(default_options_t options, tool_config_t config);

void default_init(void);

END_API
