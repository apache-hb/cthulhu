#pragma once

#include "ctu/ir/ir.h"

/**
 * ir optimization
 * hence called speed
 */

bool remove_dead_code(module_t *mod);

/**
 * remove unreferenced block labels
 */
bool remove_unused_blocks(module_t *mod);
