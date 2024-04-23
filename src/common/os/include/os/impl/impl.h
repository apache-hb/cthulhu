// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "core/compiler.h"

#if CT_OS_WINDOWS
#   include "win32.h" // IWYU pragma: export
#elif CT_OS_LINUX || CT_OS_APPLE
#   include "posix.h" // IWYU pragma: export
#else
#   error "unsupported platform"
#endif
