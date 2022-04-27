#pragma once

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/util/map.h"
#include "cthulhu/util/report.h"

typedef uint64_t offset_t;
typedef uint32_t type_t;
typedef uint32_t magic_t;
typedef uint32_t submagic_t;
typedef uint32_t semver_t;
typedef uint8_t bool_t;

typedef enum { FIELD_STRING,
               FIELD_INT,
               FIELD_BOOL,
               FIELD_REFERENCE,
               FIELD_ARRAY,
} field_t;

typedef struct {
    type_t type;
    offset_t offset;
} index_t;

typedef struct {
    offset_t offset;
    offset_t length;
} array_t;

#define NULL_TYPE   ((type_t)UINT32_MAX - 128)
#define NULL_OFFSET ((offset_t)UINT32_MAX - 128)

#define NULL_INDEX ((index_t) { NULL_TYPE, NULL_OFFSET })
#define NULL_ARRAY ((array_t) { UINT64_MAX, UINT64_MAX })

typedef union {
    const char *string;
    mpz_t digit;
    bool_t boolean;
    index_t reference;
    array_t array;
} value_t;

value_t string_value(const char *string);
value_t digit_value(const mpz_t digit);
value_t int_value(signed long digit);
value_t bool_value(bool boolean);
value_t reference_value(index_t reference);
value_t array_value(array_t array);

const char *get_string(value_t value);
void get_digit(mpz_t mpz, value_t value);
int64_t get_int(value_t value);
bool get_bool(value_t value);
index_t get_reference(value_t value);
array_t get_array(value_t value);

typedef struct {
    size_t length;
    const field_t *fields;
} layout_t;

typedef struct {
    const layout_t *layout;
    value_t *values;
} record_t;

typedef struct {
    size_t types;
    const layout_t *layouts;
} format_t;

#define FIELD(name, type) type
#define LAYOUT(name, array) \
    { .length = sizeof(array) / sizeof(field_t), .fields = (array) }
#define FORMAT(array) \
    { .types = sizeof(array) / sizeof(layout_t), .layouts = (array) }

#define FIELDLEN(name) (sizeof(name) / sizeof(field_t))

#define NEW_VERSION(major, minor, patch) (((major) << 24) | ((minor) << 16) | (patch))

#define VERSION_MAJOR(version) (((version) >> 24) & 0xFF)
#define VERSION_MINOR(version) (((version) >> 16) & 0xFF)
#define VERSION_PATCH(version) ((version)&0xFFFF)

typedef struct {
    reports_t *reports;     // report sink
    const format_t *format; // a description of the data we're dealing with
    const char *path;       // where is this data
    submagic_t submagic;    // submagic for this data
    semver_t semver;        // semver for this data
} header_t;

typedef struct {
    header_t header;

    union {
        // data needed for writing
        struct {
            stream_t *stream;   // the global stream that everything eventually
                                // ends up in
            stream_t *strings;  // the string table
            map_t *cache;       // string offset cache
            stream_t *arrays;   // the array table
            stream_t **records; // all our serialized data per type
        };

        // data needed for reading
        struct {
            const char *data; // the raw data
            size_t length;    // the length of the data
            size_t string;    // string table offset
            size_t array;     // array table offset
            size_t *offsets;  // the offset table
        };
    };

    size_t *counts; // the amount of records per type
    size_t *sizes;  // the size of each type
} data_t;

void begin_save(data_t *out, header_t header);
void end_save(data_t *out);

index_t write_entry(data_t *out, type_t type, const value_t *values);
array_t write_array(data_t *out, index_t *indices, size_t len);

bool begin_load(data_t *in, header_t header);
void end_load(data_t *in);

bool read_entry(data_t *in, index_t index, value_t *values);
bool read_array(data_t *in, array_t array, index_t *indices);
