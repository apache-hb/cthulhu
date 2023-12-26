#pragma once

#include "core/compiler.h"
#include "core/version_def.h"

#include <stdbool.h>

BEGIN_API

#if OS_WINDOWS
#   define DISPLAY_WIN_STYLE true
#else
#   define DISPLAY_WIN_STYLE false
#endif

/// @defgroup Display Pretty message printing
/// @ingroup Common
/// @brief Text formatting for displaying types to the user
/// @{

typedef struct colour_pallete_t colour_pallete_t;
typedef struct io_t io_t;
typedef struct arena_t arena_t;
typedef struct config_t config_t;

/// @brief basic format options
/// format options that all formatting functions require
typedef struct display_options_t
{
    /// @brief the arena to allocate from for temporary buffers
    /// no memory allocated from this arena will be retained
    /// after the function returns.
    arena_t *arena;

    /// @brief the io buffer to write to
    /// @note formatters will write to this buffer multiple times
    ///       so it should be able to handle multiple writes.
    /// @note the buffer must have been created with @a eAccessWrite and @a eAccessText
    /// this buffer will never be read from.
    io_t *io;

    /// @brief the colour pallete to use
    /// it is up to the formatter what colours from the pallete are used.
    /// @note it is up to the caller to ensure that the pallete and io buffer
    ///       are compatible.
    const colour_pallete_t *colours;
} display_options_t;

/// @brief version info format options
typedef struct version_display_t
{
    /// @brief basic format options
    display_options_t options;

    /// @brief the version info to format
    version_info_t version;

    /// @brief the name of the program
    const char *name;
} version_display_t;

/// @brief config object format options
typedef struct config_display_t
{
    /// @brief basic format options
    display_options_t options;

    /// @brief the config object to format
    const config_t *config;

    /// @brief should the command line usage header be printed
    bool print_usage;

    /// @brief which platform to format for
    /// if true, all flags will be formatted with a leading slash
    /// if false, short flags will be formatted with a leading dash and long flags with a leading double dash
    bool win_style;

    /// @brief command line name of this program
    /// @note can be null if @a print_usage is false
    const char *name;
} config_display_t;

/// @brief format a version info object to text
/// formats simmilar to the output of `--version` in GNU programs
///
/// @param options the formatting options
void display_version(version_display_t options);

/// @brief format a config object to text
/// formats simmilar to the output of `--help` in GNU programs
///
/// @param options the formatting options
void display_config(config_display_t options);

/// @}

END_API
