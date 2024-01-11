#pragma once

#include "format/format.h"

#include <stdbool.h>

BEGIN_API

typedef struct cfg_group_t cfg_group_t;

/// @brief config format options
typedef struct print_config_t
{
    print_options_t options;

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
} print_config_t;

void print_config(print_config_t config);

END_API