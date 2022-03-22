#include "common.h"

static size_t field_size(field_t field) {
    switch (field) {
    case FIELD_STRING: return sizeof(size_t);
    case FIELD_INT: return sizeof(size_t);
    case FIELD_BOOL: return sizeof(bool);
    case FIELD_REFERENCE: return sizeof(index_t);
    default: return 0;
    }
}

size_t layout_size(layout_t layout) {
    size_t result = 0;

    for (size_t i = 0; i < layout.length; i++) {
        result += field_size(layout.fields[i]);
    }

    return result;
}

void begin_data(data_t *data, header_t header) {
    size_t len = header.format->types;

    data->header = header;

    data->sizes = ctu_malloc(sizeof(size_t) * len);

    for (size_t i = 0; i < len; i++) {
        data->sizes[i] = layout_size(header.format->layouts[i]);
    }
}
