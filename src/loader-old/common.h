#include "cthulhu/loader/loader.h"
#include <errno.h>

/**
 * serialized file layout
 * 
 * header:
 *  - magic
 *  - version
 *  - spans
 *  - nodes
 *  - counts
 * nodes:
 *  - array of spans
 *  - array of strings
 *  - array of sparse arrays
 *  - array of node data
 */

#define NEW_VERSION(major, minor, patch) ((major << 24) | (minor << 16) | patch)

#define VERSION_MAJOR(version) ((version >> 24) & 0xFF)
#define VERSION_MINOR(version) ((version >> 16) & 0xFF)
#define VERSION_PATCH(version) (version & 0xFFFF)

#define HEADER_MAGIC 0xB00B
#define CURRENT_VERSION NEW_VERSION(1, 0, 0)

STATIC_ASSERT(HLIR_TOTAL <= UINT8_MAX, "hlir_t has too many types");

#define STRUCT_ALIGN 2
#define DIGIT_BASE 26
#define TOTAL_COUNTS ROUND2(HLIR_TOTAL, 2)

BEGIN_PACKED(STRUCT_ALIGN)

typedef uint32_t offset_t;
typedef uint32_t length_t;

typedef struct PACKED(STRUCT_ALIGN) {
    uint32_t magic;
    uint32_t version;

    offset_t language;

    offset_t strings; // absolute offset of the string table
    offset_t spans; // absolute offset of the span table
    offset_t arrays; // absolute offset of the sparse array table
    offset_t offsets[TOTAL_COUNTS]; // node offsets
    length_t counts[TOTAL_COUNTS]; // node counts
} header_t;

typedef struct PACKED(STRUCT_ALIGN) {
    offset_t offset; // absolute offset from start of file
    length_t count; // number of entries in this array
} array_t;

typedef struct PACKED(STRUCT_ALIGN) {
    array_t spans; // where the span data is 
    array_t nodes[TOTAL_COUNTS]; // where each type of node data is 
} counts_t;

typedef struct PACKED(STRUCT_ALIGN) {
    uint8_t type; // index into counts.nodes
    length_t index; // which node from the array to use
} node_index_t;

typedef struct PACKED(STRUCT_ALIGN) {
    length_t size;
    node_index_t offsets[];
} node_array_t;

typedef struct PACKED(STRUCT_ALIGN) {
    offset_t span;
    node_index_t type;
} node_header_t;

// HLIR_DIGIT_LITERAL
typedef struct PACKED(STRUCT_ALIGN) {
    node_header_t header;
    offset_t digit;
} digit_node_t;

// HLIR_BOOL_LITERAL
typedef struct PACKED(STRUCT_ALIGN) {
    node_header_t header;
    bool value;
} bool_node_t;

// HLIR_STRING_LITERAL
typedef struct PACKED(STRUCT_ALIGN) {
    node_header_t header;
    offset_t string;
} string_node_t;

// HLIR_NAME
// HLIR_UNARY
// HLIR_BINARY
// HLIR_COMPARE
// HLIR_CALL
// HLIR_ARRAY_INIT

// HLIR_STMTS
// HLIR_BRANCH
// HLIR_LOOP
// HLIR_ASSIGN

// HLIR_STRUCT
// HLIR_UNION

// HLIR_DIGIT
typedef struct PACKED(STRUCT_ALIGN) {
    node_header_t header;
    offset_t name;
    digit_t width;
    sign_t sign;
} digit_type_node_t;

// HLIR_BOOL
// HLIR_STRING
// HLIR_VOID
// HLIR_CLOSURE
// HLIR_POINTER
// HLIR_ARRAY
// HLIR_TYPE
typedef struct PACKED(STRUCT_ALIGN) {
    node_header_t header;
    offset_t name;
} metatype_node_t;

// HLIR_ALIAS

// HLIR_FORWARD
// HLIR_FUNCTION

// HLIR_VALUE
typedef struct PACKED(STRUCT_ALIGN) {
    node_header_t header;
    offset_t name;
    node_index_t init;
} value_node_t;

// HLIR_MODULE
typedef struct PACKED(STRUCT_ALIGN) {
    node_header_t header;
    offset_t name;
    offset_t globals;
    offset_t types;
} module_node_t;

// HLIR_FIELD

// HLIR_ERROR

size_t get_node_size(reports_t *reports, uint8_t type);
size_t get_node_count(reports_t *reports, length_t bytes, uint8_t type);

END_PACKED
