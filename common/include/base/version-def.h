#pragma once

#include <stdint.h>
#include <inttypes.h>

#define NEW_VERSION(major, minor, patch) (((major) << 24) | ((minor) << 16) | (patch))

#define VERSION_MAJOR(version) (((version) >> 24) & 0xFF)
#define VERSION_MINOR(version) (((version) >> 16) & 0xFF)
#define VERSION_PATCH(version) ((version)&0xFFFF)

typedef uint_fast32_t version_t;

#define PRI_VERSION PRIuFAST32

typedef struct version_info_t {
    const char *license;
    const char *desc;
    const char *author;

    version_t version;
} version_info_t;
