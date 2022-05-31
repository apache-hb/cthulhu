#pragma once

#include "cthulhu/interface/interface.h"

///
/// code required by drivers
/// this is provided by the generic framework
///

/**
 * @brief find a module by name
 *
 * @param runtime the runtime to take the module from
 * @param path the name of the module
 * @return sema_t* a module if one was found otherwise NULL
 */
sema_t *find_module(runtime_t *runtime, const char *path);

void add_module(runtime_t *runtime, const char *name, sema_t *sema);
