#pragma once

#include "core/version_def.h"

#include "core/analyze.h"
#include "format/format.h"

BEGIN_API

/// @defgroup format_version Format version information
/// @ingroup format
/// @brief Format version information
/// @{

/// @brief version formatting config
typedef struct print_version_t
{
    /// @brief the options to use when printing
    print_options_t options;
} print_version_t;

/// @brief print the version of the program
///
/// @param config the config to use when printing
/// @param version the version to print
/// @param name the name of the program
void print_version(print_version_t config, version_info_t version, IN_STRING const char *name);

/// @}

END_API
