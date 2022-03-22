#include "common.h"

#include <string.h>
#include <errno.h>

static offset_t write_string(stream_t *vec, const char *str) {
    size_t len = strlen(str) + 1;
    size_t result = stream_len(vec);

    stream_write_bytes(vec, str, len);

    return (offset_t)result;
}

static void write_data(stream_t *strings, stream_t *dst, const record_t *record) {
    size_t len = record->layout->length;

    for (size_t i = 0; i < len; i++) {
        field_t field = record->layout->fields[i];
        value_t val = record->values[i];

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
        }
    }
}

void begin_save(data_t *out, header_t header) {
    begin_data(out, header);

    size_t len = header.format->types;

    out->strings = stream_new(0x1000);

    out->records = ctu_malloc(sizeof(stream_t*) * len);
    out->counts = ctu_malloc(sizeof(size_t) * len);

    for (size_t i = 0; i < len; i++) {
        out->records[i] = stream_new(0x1000);
        out->counts[i] = 0;
    }
}

void end_save(data_t *out) {
    stream_t *header = stream_new(0x100);
    magic_t magic = FILE_MAGIC;
    header_t config = out->header;
    size_t len = config.format->types;

    // write our basic header
    stream_write_bytes(header, &magic, sizeof(magic_t));
    stream_write_bytes(header, &config.submagic, sizeof(submagic_t));
    stream_write_bytes(header, &config.semver, sizeof(semver_t));

    // write user header
    write_data(out->strings, header, &config.header);

    // write the count array
    for (size_t i = 0; i < len; i++) {
        stream_write_bytes(header, &out->counts[i], sizeof(offset_t));
    }

    stream_write(out->strings, "");
    size_t nstrings = stream_len(out->strings);
    size_t ntotal = stream_len(header) + (sizeof(offset_t) * len);
    size_t offset = ntotal + nstrings;

    // write the offset array
    size_t cursor = offset;
    for (size_t i = 0; i < len; i++) {
        stream_write_bytes(header, &cursor, sizeof(offset_t));
        cursor += (out->counts[i] * out->sizes[i]);
    }

    stream_write_bytes(header, stream_data(out->strings), nstrings);

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

index_t write_entry(data_t *out, type_t type, const record_t *record) {
    stream_t *dst = out->records[type];

    write_data(out->strings, dst, record);

    offset_t offset = out->counts[type]++;
    index_t index = { type, offset };

    return index;
}
