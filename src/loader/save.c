#include "common.h"

#include <string.h>
#include <errno.h>

static offset_t write_string(stream_t *vec, const char *str) {
    size_t len = strlen(str) + 1;
    size_t result = stream_len(vec);

    stream_write_bytes(vec, str, len);

    return result;
}

static void write_data(stream_t *strings, stream_t *dst, layout_t layout, const value_t *values) {
    size_t len = layout.length;

    for (size_t i = 0; i < len; i++) {
        field_t field = layout.fields[i];
        value_t val = values[i];

        offset_t str;
        char *gmp;
        
        switch (field) {
        case FIELD_STRING:
            str = write_string(strings, val.string);
            stream_write_bytes(dst, &str, sizeof(offset_t));
            break;
        case FIELD_INT:
            gmp = mpz_get_str(NULL, DIGIT_BASE, val.digit);
            str = write_string(strings, gmp);
            stream_write_bytes(dst, &str, sizeof(offset_t));
            break;
        case FIELD_BOOL:
            stream_write_bytes(dst, &val.boolean, sizeof(bool_t));
            break;
        case FIELD_REFERENCE:
            stream_write_bytes(dst, &val.reference, sizeof(index_t));
            break;
        case FIELD_ARRAY:
            stream_write_bytes(dst, &val.array, sizeof(array_t));
            break;
        }
    }
}

void begin_save(data_t *out, header_t header) {
    begin_data(out, header);

    size_t len = header.format->types;

    out->strings = stream_new(0x1000);
    out->arrays = stream_new(0x1000);
    out->stream = stream_new(0x1000);

    out->records = ctu_malloc(sizeof(stream_t*) * len);
    out->counts = ctu_malloc(sizeof(size_t) * len);

    for (size_t i = 0; i < len; i++) {
        out->records[i] = stream_new(0x1000);
        out->counts[i] = 0;
    }

    write_data(out->strings, out->stream, *header.header.layout, header.header.values);
}

void end_save(data_t *out) {
    stream_t *header = stream_new(0x100);
    header_t config = out->header;
    size_t len = config.format->types;
    size_t subheader = layout_size(*config.header.layout);

    stream_write(out->strings, "");
    size_t nstrings = stream_len(out->strings);
    size_t narrays = stream_len(out->arrays);
    size_t ncounts = (sizeof(offset_t) * len);
    size_t noffsets = (sizeof(offset_t) * len);
    size_t nheader = sizeof(basic_header_t) + subheader;
    size_t offset = nheader + noffsets + ncounts + nstrings + narrays;

    basic_header_t basic = {
        .magic = FILE_MAGIC,
        .submagic = config.submagic,
        .semver = config.semver,
        .strings = nheader + noffsets + ncounts,
        .arrays = nheader + noffsets + ncounts + nstrings,
    };

    logverbose("header(strings=%llu,arrays=%llu)", basic.strings, basic.arrays);

    // write our basic header
    stream_write_bytes(header, &basic, sizeof(basic_header_t));

    // write user header
    stream_write_bytes(header, stream_data(out->stream), stream_len(out->stream));

    // write the count array
    for (size_t i = 0; i < len; i++) {
        stream_write_bytes(header, &out->counts[i], sizeof(offset_t));
    }

    // write the offset array
    size_t cursor = offset;
    for (size_t i = 0; i < len; i++) {
        stream_write_bytes(header, &cursor, sizeof(offset_t));
        cursor += (out->counts[i] * out->sizes[i]);
    }

    // write string data and array data
    stream_write_bytes(header, stream_data(out->strings), nstrings);
    stream_write_bytes(header, stream_data(out->arrays), narrays);

    for (size_t i = 0; i < len; i++) {
        size_t size = out->counts[i] * out->sizes[i];
        stream_write_bytes(header, stream_data(out->records[i]), size);
    }

    file_t file = ctu_fopen(out->header.path, "wb");

    if (!file_valid(&file)) {
        report(out->header.reports, ERROR, NULL, "failed to open file %s (errno %d)", out->header.path, errno);
        return;
    }

    fwrite(stream_data(header), 1, stream_len(header), file.file);

    ctu_close(&file);
}

index_t write_entry(data_t *out, type_t type, const value_t *values) {
    stream_t *dst = out->records[type];
    layout_t layout = out->header.format->layouts[type];

    write_data(out->strings, dst, layout, values);

    offset_t offset = out->counts[type]++;
    index_t index = { type, offset };

    return index;
}

array_t write_array(data_t *out, index_t *indices, size_t len) {
    array_t result = {
        .length = len,
        .offset = stream_len(out->arrays)
    };

    stream_write_bytes(out->arrays, indices, sizeof(index_t) * len);
    
    return result;
}
