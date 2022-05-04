#pragma once

#include "cthulhu/util/macros.h"
#include "cthulhu/util/version-def.h"
#include "version.h"

/**
 * file layout
 *
 * - header: header_t
 * - offset table: uint64_t[]
 * - string table: char[]
 *   - array of strings, each string is null-terminated
 * - array table: entry_t[]
 * - data blobs
 */

#define PACKING 2

BEGIN_PACKED(PACKING)

typedef struct PACKED(PACKING)
{
    uint64_t magic;    // global magic number
    version_t version; // version of the file format
    uint32_t submagic; // content specific magic number
    uint32_t strings;  // total size of the string table
} header_t;

typedef struct PACKED(PACKING)
{
    uint32_t index;  // index into the array table
    uint32_t length; // number of elements to read
} array_t;

typedef struct PACKED(PACKING)
{
    uint32_t type;  // index into offset table
    uint32_t index; // number of elements into the offset table
} entry_t;

END_PACKED()

typedef enum
{
    TYPE_STRING,
    TYPE_ARRAY,
    TYPE_ENTRY,
    TYPE_BOOL,
    TYPE_INT
} element_t;

typedef struct
{
    size_t size;
    const element_t *elements;
} layout_t;

typedef struct
{
    uint32_t submagic;

    size_t size;
    const layout_t *layouts;
} format_t;

#define ENTRY(name, type) type
#define LAYOUT(name, elements)                                                                                         \
    {                                                                                                                  \
        .size = sizeof(elements) / sizeof(element_t), .elements = elements                                             \
    }
#define FORMAT(submagic, layouts)                                                                                      \
    {                                                                                                                  \
        .submagic = submagic, .size = sizeof(layouts) / sizeof(layout_t), .layouts = layouts                           \
    }

typedef struct
{
    format_t spec;

    uint32_t *sizes;

    stream_t *arrayData;
    stream_t *stringData;

    stream_t **dataTables;
} save_t;

typedef struct
{
    format_t spec;

    uint32_t *sizes;

    uint64_t *offsets;
    stream_t *stringData;
    stream_t *arrayData;

    stream_t *tableData;
} load_t;

STATIC_ASSERT(sizeof(header_t) == 20, "header_t is not 20 bytes");
STATIC_ASSERT(sizeof(array_t) == 8, "array_t is not 8 bytes");
STATIC_ASSERT(sizeof(entry_t) == 8, "entry_t is not 8 bytes");

void begin_save(save_t *save, format_t spec);
void end_save(save_t *save, file_t file);

void begin_load(load_t *load, format_t spec, file_t file);
void end_load(load_t *load);
