// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_config.h>

/// @defgroup win32 Windows wrapper
/// @brief Windows wrapper header, use this instead of including windows directly
/// @ingroup core
/// @{

#ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#   define NOMINMAX
#endif

#ifndef STRICT
#   define STRICT
#endif

#ifndef VC_EXTRALEAN
#   define VC_EXTRALEAN
#endif

#include <windows.h> // IWYU pragma: export

/// @}
