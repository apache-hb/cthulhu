#pragma once

/// @defgroup win32 Windows wrapper
/// @brief Windows wrapper header, use this instead of including windows directly
/// @ingroup core
/// @{

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define STRICT
#define VC_EXTRALEAN

#include <windows.h> // IWYU pragma: export

/// @}
