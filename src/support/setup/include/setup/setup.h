#pragma once

#include <ctu_setup_api.h>

#include "core/compiler.h"
#include "core/version_def.h"

#include <stdbool.h>

BEGIN_API

typedef struct cfg_group_t cfg_group_t;
typedef struct cfg_field_t cfg_field_t;
typedef struct io_t io_t;
typedef struct arena_t arena_t;
typedef struct ap_t ap_t;

/// @defgroup setup Default options
/// @brief Default command line options and behaviour
/// @ingroup support
/// @{

/// @brief tool config
typedef struct tool_config_t
{
    /// @brief the arena to use
    arena_t *arena;

    /// @brief the io buffer to use
    io_t *io;

    /// @brief the root config group
    cfg_group_t *group;

    /// @brief this tools version
    version_info_t version;

    /// @brief the number of arguments
    int argc;

    /// @brief the arguments
    const char **argv;
} tool_config_t;

/// @brief default options
typedef struct default_options_t
{
    // default config group
    cfg_group_t *general_group;

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

    // debug config group
    cfg_group_t *debug_group;

    // enable debug verbosity
    cfg_field_t *log_verbose;
} default_options_t;

/// @brief get the default options
///
/// @param group the config group to use
///
/// @return the default options
CT_SETUP_API default_options_t get_default_options(cfg_group_t *group);

/// @brief process the default options
/// @note if this function does not return @a CT_EXIT_OK, the program should exit
///       with the returned error code
///
/// @param options the default options
/// @param config the tool config
///
/// @return @a CT_EXIT_OK on success or an error code
CT_SETUP_API int process_default_options(default_options_t options, tool_config_t config);

/// @brief parse the default commands
/// @note if this function does not return @a CT_EXIT_OK, the program should exit
///       with the returned error code
///
/// @param options the default options
/// @param config the tool config
///
/// @return @a CT_EXIT_OK on success or an error code
CT_SETUP_API int parse_commands(default_options_t options, tool_config_t config);

/// @brief parse the default arguments
/// @note this function is the same as @see parse_commands but allows
///       the user to use their own @see ap_t instance
///
/// @param ap the parser instance
/// @param options the default options
/// @param config the tool config
///
/// @return @a CT_EXIT_OK on success or an error code
CT_SETUP_API int parse_argparse(ap_t *ap, default_options_t options, tool_config_t config);

/// @brief initialise the runtime with default options
CT_SETUP_API void setup_global(void);

/// @}

END_API
