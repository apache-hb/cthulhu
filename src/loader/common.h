#include "cthulhu/loader/loader.h"
#include <errno.h>

/**
 * serialized file layout
 * 
 * header:
 *  - magic
 *  - version
 * counts:
 *  - spans
 *  - nodes
 * nodes:
 *  - arrays of node data
 */

#define NEW_VERSION(major, minor, patch) ((major << 24) | (minor << 16) | patch)

#define VERSION_MAJOR(version) ((version >> 24) & 0xFF)
#define VERSION_MINOR(version) ((version >> 16) & 0xFF)
#define VERSION_PATCH(version) (version & 0xFFFF)

#define HEADER_MAGIC 0xB00B
#define CURRENT_VERSION NEW_VERSION(1, 0, 0)

STATIC_ASSERT(HLIR_TOTAL <= UINT8_MAX, "hlir_t has too many types");

#define DIGIT_BASE 26

BEGIN_PACKED

typedef uint32_t offset_t;

typedef struct PACKED {
    uint32_t magic;
    uint32_t version;

    offset_t strings; // absolute offset of the string table
    offset_t spans; // absolute offset of the span table
    offset_t counts[HLIR_TOTAL]; // node counters
} header_t;

typedef struct PACKED {
    offset_t offset; // absolute offset from start of file
    offset_t count; // number of entries in this array
} array_t;

STATIC_ASSERT(sizeof(array_t) == (2 * sizeof(offset_t)), "array_t must be 8 bytes");

typedef struct PACKED {
    array_t spans; // where the span data is 
    array_t nodes[HLIR_TOTAL]; // where each type of node data is 
} counts_t;

STATIC_ASSERT(sizeof(counts_t) == sizeof(array_t) * HLIR_TOTAL + sizeof(array_t), "counts_t must be 4 * HLIR_TOTAL bytes");

typedef struct PACKED {
    uint8_t type; // index into counts.nodes
    offset_t index; // which node from the array to use
} node_index_t;

STATIC_ASSERT(sizeof(node_index_t) == (sizeof(uint8_t) + sizeof(offset_t)), "node_index_t must be 8 bytes");

typedef struct PACKED {
    uint32_t size;
    node_index_t offsets[];
} node_array_t;

typedef struct PACKED {
    offset_t span;
    node_index_t type;
} node_header_t;

STATIC_ASSERT(sizeof(node_header_t) == sizeof(offset_t) + sizeof(node_index_t), "node_header_t must be 4 bytes");

typedef struct PACKED {
    node_header_t header;
    offset_t string;
} string_node_t;

typedef struct PACKED {
    node_header_t header;
    offset_t digit;
} digit_node_t;

typedef struct PACKED {
    node_header_t header;
    bool value;
} bool_node_t;

typedef struct PACKED {
    node_header_t header;
    offset_t name;
    node_index_t init;
} value_node_t;

typedef struct PACKED {
    node_header_t header;
    offset_t globals;
    offset_t types;
} module_node_t;

typedef struct PACKED {
    node_header_t header;
    offset_t name;
    digit_t width;
    sign_t sign;
} digit_type_node_t;

typedef struct PACKED {
    node_header_t header;
    offset_t name;
} metatype_node_t;

END_PACKED
