#pragma once

#include "cthulhu/util/report.h"
#include "cthulhu/hlir/hlir.h"

typedef uint64_t offset_t;
typedef uint32_t type_t;
typedef uint32_t magic_t;
typedef uint32_t submagic_t;
typedef uint32_t semver_t;
typedef uint8_t bool_t;

typedef enum {
    FIELD_STRING,
    FIELD_INT,
    FIELD_BOOL,
    FIELD_REFERENCE,
    FIELD_ARRAY
} field_t;

typedef struct {
    type_t type;
    offset_t offset;
} index_t;

typedef union {
    const char *string;
    mpz_t digit;
    bool_t boolean;
    index_t reference;
} value_t;

typedef struct layout_t {
    size_t length;
    const field_t *fields;
} layout_t;

typedef struct record_t {
    const layout_t *layout;
    value_t *values;
} record_t;

typedef struct {
    layout_t header;

    size_t types;
    const layout_t *layouts;
} format_t;

#define FIELD(name, type) type
#define LAYOUT(name, array) { .length = sizeof(array) / sizeof(field_t), .fields = array }
#define FORMAT(head, array) { .header = head, .types = sizeof(array) / sizeof(layout_t), .layouts = array }

#define NEW_VERSION(major, minor, patch) ((major << 24) | (minor << 16) | patch)

#define VERSION_MAJOR(version) ((version >> 24) & 0xFF)
#define VERSION_MINOR(version) ((version >> 16) & 0xFF)
#define VERSION_PATCH(version) (version & 0xFFFF)

typedef struct {
    reports_t *reports; // report sink
    const format_t *format; // a description of the data we're dealing with
    const char *path; // where is this data
    record_t header; // our expected data header
    submagic_t submagic; // submagic for this data
    semver_t semver; // semver for this data
} header_t;

typedef struct {
    header_t header;

    union {
        // data needed for writing
        struct {
            stream_t *strings; // the string table
            stream_t **records; // all our serialized data per type
        };

        // data needed for reading
        struct {
            const char *data; // the raw data
            size_t string; // string table offset
            size_t *offsets; // the offset table
        };
    };

    size_t *counts; // the amount of records per type
    size_t *sizes; // the size of each type
} data_t;

void begin_save(data_t *out, header_t header);
void end_save(data_t *out);

index_t write_entry(data_t *out, type_t type, const record_t *record);

bool begin_load(data_t *in, header_t header);
void end_load(data_t *in);

bool read_entry(data_t *in, index_t index, record_t *record);
