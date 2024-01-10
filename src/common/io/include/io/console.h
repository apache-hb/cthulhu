#pragma once

#include "core/compiler.h"

BEGIN_API

typedef struct io_t io_t;

io_t *io_stdout(void);
io_t *io_stderr(void);

END_API
