#include "common.h"

#include <string.h>

void begin_save(data_t *out, reports_t *reports, const format_t *format, const char *path) {
    begin_data(out, reports, format, path);
    
    size_t len = format->types;

    out->strings = stream_new(0x1000);

    out->records = ctu_malloc(sizeof(stream_t*) * len);
    out->counts = ctu_malloc(sizeof(size_t) * len);

    for (size_t i = 0; i < len; i++) {
        out->records[i] = stream_new(0x1000);
        out->counts[i] = 0;
    }
}

void end_save(data_t *out) {

}

static size_t write_string(stream_t *vec, const char *str) {
    size_t len = strlen(str) + 1;
    size_t result = stream_len(vec);

    stream_write_bytes(vec, str, len);

    return result;
}

index_t write_entry(data_t *out, size_t type, const record_t *record) {
    size_t len = record->layout->length;
    stream_t *dst = out->records[type];

    for (size_t i = 0; i < len; i++) {
        field_t field = record->layout->fields[i];
        value_t val = record->values[i];

        size_t str;
        char *gmp;
        switch (field) {
        case FIELD_STRING:
            str = write_string(out->strings, val.string);
            stream_write_bytes(dst, &str, sizeof(size_t));
            break;
        case FIELD_INT:
            gmp = mpz_get_str(NULL, DIGIT_BASE, val.digit);
            str = write_string(out->strings, gmp);
            stream_write_bytes(dst, &str, sizeof(size_t));
            break;
        case FIELD_BOOL:
            stream_write_bytes(dst, &val.boolean, sizeof(bool));
            break;
        case FIELD_REFERENCE:
            stream_write_bytes(dst, &val.reference, sizeof(index_t));
            break;
        }
    }

    size_t offset = out->counts[type]++;
    index_t index = { type, offset };

    return index;
}
