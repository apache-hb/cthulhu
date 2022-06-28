#pragma once

#include <stdint.h>

#define NEW_VERSION(major, minor, patch) (((major) << 24) | ((minor) << 16) | (patch))

#define VERSION_MAJOR(version) (((version) >> 24) & 0xFF)
#define VERSION_MINOR(version) (((version) >> 16) & 0xFF)
#define VERSION_PATCH(version) ((version)&0xFFFF)

typedef uint_fast32_t version_t;

#define PRI_VERSION PRIuFAST32
