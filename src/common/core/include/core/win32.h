#pragma once

#include <ctu_config.h>

/// @defgroup win32 Windows wrapper
/// @brief Windows wrapper header, use this instead of including windows directly
/// @ingroup core
/// @{

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define STRICT
#define VC_EXTRALEAN

#if CTU_WIN32_TRICKERY
// detect amd64, x86, or arm and define them ourselves
#   if defined(_M_IX86)
#       define _X86_
#   elif defined(_M_AMD64)
#       define _AMD64_
#   elif defined(_M_ARM)
#       define _ARM_
#   elif defined(_M_ARM64)
#       define _ARM64_
#   else
#       error "Unknown architecture, disable win32 trickery or add your arch here"
#   endif
#else
#   include <windows.h> // IWYU pragma: export
#endif

/// @}
