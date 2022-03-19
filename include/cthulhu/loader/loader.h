#pragma once

#include "cthulhu/util/report.h"
#include "cthulhu/hlir/hlir.h"

/**
 * @brief attempt to load a module from a given file
 * 
 * @param reports report sink
 * @param path the path to the file
 * @return hlir_t* the module if it was found
 */
hlir_t *load_module(reports_t *reports, const char *path);

/**
 * @brief save a module to a file
 * 
 * @param reports report sink
 * @param module the module to save
 * @param path where to save the module
 */
void save_module(reports_t *reports, hlir_t *module, const char *path);

typedef enum {
    FIELD_STRING,
    FIELD_INT,
    FIELD_BOOL,
    FIELD_REFERENCE
} field_t;

typedef struct {
    size_t type;
    size_t offset;
} index_t;

typedef union {
    const char *string;
    mpz_t digit;
    bool boolean;
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
    size_t types;
    const layout_t *layouts;
} format_t;

#define LAYOUT(array) { .length = sizeof(array) / sizeof(field_t), .fields = array }
#define FORMAT(array) { .types = sizeof(array) / sizeof(layout_t), .layouts = array }

typedef struct {
    reports_t *reports; // report sink
    const format_t *format; // a description of the data we're dealing with
    const char *path; // where is this data

    stream_t *strings; // the string table
    stream_t **records; // all our serialized data per type
    size_t *counts; // the amount of records per type

    size_t *sizes; // the size of each type
} data_t;

void begin_save(data_t *out, reports_t *reports, const format_t *format, const char *path);
void end_save(data_t *out);

index_t write_entry(data_t *out, size_t type, const record_t *record);


void begin_load(data_t *in, reports_t *reports, const format_t *format, const char *path);
void end_load(data_t *in);

bool read_entry(data_t *in, index_t index, record_t *record);
