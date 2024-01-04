#pragma once

#include "format/colour.h"

#include <stdbool.h>

BEGIN_API

#if OS_WINDOWS
#   define DISPLAY_WIN_STYLE true
#else
#   define DISPLAY_WIN_STYLE false
#endif

typedef struct io_t io_t;
typedef struct cfg_group_t cfg_group_t;

/// @brief config format options
typedef struct format_config_t
{
    /// @brief basic format options
    format_context_t context;

    /// @brief the io buffer to write to
    io_t *io;

    /// @brief the config object to format
    const cfg_group_t *config;

    /// @brief should the command line usage header be printed
    bool print_usage;

    /// @brief which platform to format for
    /// if true, all flags will be formatted with a leading slash
    /// if false, short flags will be formatted with a leading dash and long flags with a leading double dash
    bool win_style;

    /// @brief command line name of this program
    /// @note can be null if @a print_usage is false
    const char *name;
} format_config_t;

void print_config(format_config_t config);

END_API
