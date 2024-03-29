// SPDX-License-Identifier: LGPL-3.0-only

#ifndef LOADER_MODULE
#   define LOADER_MODULE(ID, STR, BIT)
#endif

/// module types

LOADER_MODULE(eModNone, "none", 0)
LOADER_MODULE(eModLanguage, "language", (1 << 0))
LOADER_MODULE(eModTarget, "target", (1 << 1))
LOADER_MODULE(eModPlugin, "plugin", (1 << 2))

#undef LOADER_MODULE

#ifndef LOADER_ERROR
#   define LOADER_ERROR(ID, STR)
#endif

LOADER_ERROR(eLoadErrorNone, "no error")
LOADER_ERROR(eLoadErrorLibrary, "issue loading the library")
LOADER_ERROR(eLoadErrorNoEntry, "no entry point found")
LOADER_ERROR(eLoadErrorDisabled, "loader type disabled")

#undef LOADER_ERROR

#ifndef LOADER_FEATURE
#   define LOADER_FEATURE(ID, STR, BIT)
#endif

LOADER_FEATURE(eLoadNone, "none", 0)
LOADER_FEATURE(eLoadStatic, "static", (1 << 0))
LOADER_FEATURE(eLoadDynamic, "dynamic", (1 << 1))

#undef LOADER_FEATURE
