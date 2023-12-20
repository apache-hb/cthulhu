#pragma once

#include "core/compiler.h"

BEGIN_API

typedef struct io_t io_t;
typedef struct arena_t arena_t;

io_t *io_stdout(arena_t *arena);

END_API
