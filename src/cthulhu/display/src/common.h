#pragma once

#include "base/colour.h"

// TODO: merge notify display in here to make it easier to use
char *fmt_coloured2(const colour_pallete_t *colours, colour_t idx, const char *fmt, ...);
