#pragma once

#include <inttypes.h>
#include <stdint.h>

/// @defgroup core_version Version macros
/// @brief Version macros and types
/// @ingroup core
/// @{

/// @def CT_NEW_VERSION(major, minor, patch)
/// @brief creates a new version_t from @p major, @p minor and @p patch
/// @param major the major version
/// @param minor the minor version
/// @param patch the patch version
/// @return the new version_t

#define CT_NEW_VERSION(major, minor, patch) (((major) << 24) | ((minor) << 16) | (patch))

/// @def CT_VERSION_MAJOR(version)
/// @brief returns the major version of @p version
/// @param version the version to get the major version from
/// @return the major version of @p version

/// @def CT_VERSION_MINOR(version)
/// @brief returns the minor version of @p version
/// @param version the version to get the minor version from
/// @return the minor version of @p version

/// @def CT_VERSION_PATCH(version)
/// @brief returns the patch version of @p version
/// @param version the version to get the patch version from
/// @return the patch version of @p version

#define CT_VERSION_MAJOR(version) (((version) >> 24) & 0xFF)
#define CT_VERSION_MINOR(version) (((version) >> 16) & 0xFF)
#define CT_VERSION_PATCH(version) ((version)&0xFFFF)

/// @brief underlying type for version_t
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

/// @}
