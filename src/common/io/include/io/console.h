#pragma once

#include <ctu_io_api.h>

#include "core/compiler.h"

CT_BEGIN_API

typedef struct io_t io_t;

/// @ingroup io
/// @{

/// @brief get the global stdout IO object
///
/// @return the global stdout IO object
CT_IO_API io_t *io_stdout(void);

/// @brief get the global stderr IO object
///
/// @return the global stderr IO object
CT_IO_API io_t *io_stderr(void);

/// @}

CT_END_API
