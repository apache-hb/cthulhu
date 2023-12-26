#pragma once

#include "memory/arena.h"

char *fmt_align(arena_t *arena, size_t width, const char *fmt, ...);
