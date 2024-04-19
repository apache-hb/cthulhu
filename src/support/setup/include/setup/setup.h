// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "core/analyze.h"
#include "core/version_def.h"
#include "format/format.h"
#include <ctu_setup_api.h>

#include <stdbool.h>

typedef struct arena_t arena_t;
typedef struct io_t io_t;
typedef struct ap_t ap_t;
typedef struct vector_t vector_t;
typedef struct colour_pallete_t colour_pallete_t;

typedef struct cfg_group_t cfg_group_t;
typedef struct cfg_field_t cfg_field_t;

CT_BEGIN_API

/// @defgroup setup setup
/// @ingroup support
/// @brief command line parsing and setup
/// @{

/// @brief default options shared by all tools
typedef struct setup_options_t
{
    version_info_t version;
    cfg_group_t *root;
    ap_t *ap;

    /// @brief general options
    struct {
        cfg_group_t *group;

        /// @brief print help and quit
        cfg_field_t *help;

        /// @brief print version and quit
        cfg_field_t *version;
    } general;

    /// @brief diagnostic reporting options
    struct {
        cfg_group_t *group;

        /// @brief report header style
        cfg_field_t *header;

        /// @brief enable colour output
        cfg_field_t *colour;
    } report;

    /// @brief debug options. these are for debugging the compiler itself, not the user code
    struct {
        cfg_group_t *group;

        /// @brief enable verbose logging
        cfg_field_t *verbose;
    } debug;
} setup_options_t;

/// @brief the result of parsing the command line
typedef struct setup_init_t
{
    /// @brief the exitcode
    /// @warning dont use this directly, use setup_should_exit() and setup_exit_code()
    int exitcode;

    /// @brief the parsed position arguments
    vector_t *posargs;

    /// @brief the chosen colour pallete
    const colour_pallete_t *pallete;

    /// @brief the chosen heading style
    heading_style_t heading;
} setup_init_t;

/// @brief initialise the runtime with default options
CT_SETUP_API void setup_default(arena_t *arena);

/// @brief setup default options
/// @note this should be called before you've added all your options
///
/// @param info the version information
/// @param root the root config group
///
/// @return the setup options
CT_SETUP_API setup_options_t setup_options(version_info_t info, cfg_group_t *root);

/// @brief parse the command line
///
/// @param argc the number of arguments
/// @param argv the arguments
/// @param setup the setup options
///
/// @return the parsed command line
CT_SETUP_API setup_init_t setup_parse(int argc, const char **argv, setup_options_t setup);

/// accessor functions

/// @brief check if the program should exit
///
/// @param init the setup init
///
/// @return true if the program should exit
CT_SETUP_API bool setup_should_exit(IN_NOTNULL const setup_init_t *init);

/// @brief get the exit code
/// @note only call this if setup_should_exit() returns true
/// @param init the setup init
///
/// @return the exit code
CT_SETUP_API int setup_exit_code(IN_NOTNULL const setup_init_t *init);

/// @}

CT_END_API
