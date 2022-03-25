#include "common.h"

#include <string.h>

static void read_bytes(data_t *data, void *dst, size_t *cursor, size_t size) {
    memcpy(dst, data->data + *cursor, size);
    *cursor += size;
}

static char *read_string(data_t *in, size_t* cursor) {
    offset_t offset;
    read_bytes(in, &offset, cursor, sizeof(offset_t));
    if (offset == UINT64_MAX) { return NULL; }

    return ctu_strdup(in->data + in->string + offset);
}

static value_t read_value(data_t *in, field_t field, size_t *offset) {
    char *tmp;
    value_t value;

    switch (field) {
    case FIELD_STRING:
        value.string = read_string(in, offset);
        break;
    case FIELD_INT:
        tmp = read_string(in, offset);
        mpz_init_set_str(value.digit, tmp, DIGIT_BASE);
        break;
    case FIELD_BOOL:
        read_bytes(in, &value.boolean, offset, sizeof(bool_t));
        break;
    case FIELD_REFERENCE:
        read_bytes(in, &value.reference, offset, sizeof(index_t));
        break;
    case FIELD_ARRAY:
        read_bytes(in, &value.array, offset, sizeof(array_t));
        break;
    }

    return value;
}

static void read_data(data_t *in, const record_t *record, size_t offset) {
    size_t len = record->layout->length;

    for (size_t i = 0; i < len; i++) {
        record->values[i] = read_value(in, record->layout->fields[i], &offset);
    }
}

bool begin_load(data_t *in, header_t header) {
    begin_data(in, header);

    file_t file = ctu_fopen(header.path, "rb");
    if (!file_valid(&file)) { return false; }

    char *data = ctu_mmap(&file);
    in->data = data;

    offset_t cursor = 0;
    size_t len = header.format->types;

    basic_header_t basic;
    read_bytes(in, &basic, &cursor, sizeof(basic_header_t));

    in->string = basic.strings;
    in->array = basic.arrays;

    if (basic.magic != FILE_MAGIC) {
        report(header.reports, ERROR, NULL, "invalid magic number. found %x, expected %x", basic.magic, FILE_MAGIC);
        return false;
    }

    if (basic.submagic != header.submagic) {
        report(header.reports, ERROR, NULL, "invalid submagic number. found %x, expected %x", basic.submagic, header.submagic);
        return false;
    }

    if (VERSION_MAJOR(basic.semver) > VERSION_MAJOR(header.semver)) {
        report(header.reports, ERROR, NULL, "invalid major version. found %d, expected %d", VERSION_MAJOR(basic.semver), VERSION_MAJOR(header.semver));
        return false;
    }

    if (VERSION_MINOR(basic.semver) > VERSION_MINOR(header.semver)) {
        report(header.reports, ERROR, NULL, "invalid minor version. found %d, expected %d", VERSION_MINOR(basic.semver), VERSION_MINOR(header.semver));
        return false;
    }

    read_data(in, &header.header, cursor);
    cursor += layout_size(*header.header.layout);

    offset_t *counts = ctu_malloc(sizeof(offset_t) * len);
    offset_t *offsets = ctu_malloc(sizeof(offset_t) * len);

    read_bytes(in, counts, &cursor, sizeof(offset_t) * len);
    read_bytes(in, offsets, &cursor, sizeof(offset_t) * len);

    in->counts = counts;
    in->offsets = offsets;

    return true;
}

void end_load(data_t *in) {
    end_data(in);
}

bool read_entry(data_t *in, index_t index, value_t *values) {
    size_t type = index.type;
    size_t offset = in->offsets[type] + (index.offset * in->sizes[type]);
    layout_t layout = in->header.format->layouts[type];
    size_t len = layout.length;

    for (size_t i = 0; i < len; i++) {
        values[i] = read_value(in, layout.fields[i], &offset);
    }

    return true;
}

bool read_array(data_t *in, array_t array, index_t *indices) {
    memcpy(indices, in->data + in->array + array.offset, sizeof(index_t) * array.length);
    return true;
}
