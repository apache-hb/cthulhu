#include "cthulhu/loader/loader.h"
#include <errno.h>

#define NEW_VERSION(major, minor, patch) ((major << 24) | (minor << 16) | patch)

#define VERSION_MAJOR(version) ((version >> 24) & 0xFF)
#define VERSION_MINOR(version) ((version >> 16) & 0xFF)
#define VERSION_PATCH(version) (version & 0xFFFF)

#define HEADER_MAGIC 0xB00B
#define CURRENT_VERSION NEW_VERSION(1, 0, 0)

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t symbols;

    uint32_t checksum;
} header_t;

STATIC_ASSERT(sizeof(header_t) == 16, "header_t must be 16 bytes");
