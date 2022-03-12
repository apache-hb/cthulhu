#include "cthulhu/loader/loader.h"

#define HEADER_MAGIC 0xB00B

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t symbols;

    uint32_t padding;
} header_t;

STATIC_ASSERT(sizeof(header_t) == 16, "header_t must be 16 bytes");
