#pragma once

#include <stdio.h>
#include <stdbool.h>
#include "ir.h"
#include "x86-emit.h"

/**
 * unit: intermediate form
 * text: output text for debugging
 */
blob_t x64_emit_asm(unit_t *unit, bool text);
