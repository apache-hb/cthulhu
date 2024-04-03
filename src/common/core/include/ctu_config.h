#pragma once

#define CT_BUILDTYPE_SHARED 1
#define CT_BUILDTYPE_STATIC 2

#include <ctu_core_config.h>

#define CT_BUILD_SHARED (CTU_BUILDTYPE == CT_BUILDTYPE_SHARED)
#define CT_BUILD_STATIC (CTU_BUILDTYPE == CT_BUILDTYPE_STATIC)