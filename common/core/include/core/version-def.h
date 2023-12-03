#pragma once

#include <inttypes.h>
#include <stdint.h>

#define NEW_VERSION(major, minor, patch) (((major) << 24) | ((minor) << 16) | (patch))

#define VERSION_MAJOR(version) (((version) >> 24) & 0xFF)
#define VERSION_MINOR(version) (((version) >> 16) & 0xFF)
#define VERSION_PATCH(version) ((version)&0xFFFF)

typedef uint_fast32_t version_t;

#define PRI_VERSION PRIuFAST32

/// @brief version information for a driver/interface/plugin
typedef struct version_info_t
{
    const char *license; ///< the license of this component
    const char *desc;    ///< a short description of this component
    const char *author;  ///< the author of this component
    version_t version;   ///< the version info for this component
} version_info_t;
