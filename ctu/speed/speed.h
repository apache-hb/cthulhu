#pragma once

#include "ctu/ir/ir.h"

/**
 * ir optimization
 * hence called speed
 */

bool remove_dead_code(module_t *mod);

/**
 * remove empty basic blocks
 */
bool remove_empty_blocks(module_t *mod);