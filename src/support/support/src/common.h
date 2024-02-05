#pragma once

#include "support/loader.h"
#include "loader_config.h"
#include "enum_modules.h"

loaded_module_t load_error(load_error_t error, os_error_t os);
