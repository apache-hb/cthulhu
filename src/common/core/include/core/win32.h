// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_config.h>

/// @defgroup win32 Windows wrapper
/// @brief Windows wrapper header, use this instead of including windows directly
/// @ingroup core
/// @{

// undefine stuff first for unity builds
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#undef STRICT
#undef VC_EXTRALEAN

#include <windows.h> // IWYU pragma: export

/// @}
