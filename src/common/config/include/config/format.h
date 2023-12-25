#pragma once

#include "core/compiler.h"

BEGIN_API

typedef struct io_t io_t;
typedef struct config_t config_t;

/// @brief format a config object to text
/// used to print in a human readable format to show in help messages
///
/// @param config the config object to format
/// @param io the io object to write to
void config_print(const config_t *config, io_t *io);

END_API
