#pragma once

#include "cthulhu/front/front.h"

/**
 * this function pretty much is the language
 * 
 * handle all typechecking
 * handle all name resolution
 * add implicit casts where needed so the IR knows when to emit OP_CAST
 * add all type annotations where ommitted
 */
void sema_mod(nodes_t *nodes);
