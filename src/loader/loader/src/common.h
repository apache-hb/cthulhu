// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "support/loader.h"

typedef struct loader_t
{
    arena_t *arena;
} loader_t;

loaded_module_t load_error(load_error_t error, os_error_t os);
