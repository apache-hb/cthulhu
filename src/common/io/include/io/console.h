#pragma once

#include "core/compiler.h"

BEGIN_API

typedef struct io_t io_t;

/// @ingroup io
/// @{

/// @brief get the global stdout IO object
///
/// @return the global stdout IO object
io_t *io_stdout(void);

/// @brief get the global stderr IO object
///
/// @return the global stderr IO object
io_t *io_stderr(void);

/// @}

END_API
