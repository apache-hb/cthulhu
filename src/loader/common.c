#include "common.h"

static size_t field_size(field_t field) {
    switch (field) {
    case FIELD_STRING:
    case FIELD_INT:
        return sizeof(size_t);
    case FIELD_BOOL:
        return sizeof(bool_t);
    case FIELD_REFERENCE:
        return sizeof(index_t);
    case FIELD_ARRAY:
        return sizeof(array_t);
    default:
        return 0;
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

void end_data(data_t *data) {
    ctu_free(data->sizes);
}

value_t string_value(const char *string) {
    value_t result;
    result.string = string;
    return result;
}

value_t digit_value(const mpz_t digit) {
    value_t result;
    mpz_init_set(result.digit, digit);
    return result;
}

value_t int_value(signed long digit) {
    value_t result;
    mpz_init_set_si(result.digit, digit);
    return result;
}

value_t bool_value(bool boolean) {
    value_t result;
    result.boolean = boolean;
    return result;
}

value_t reference_value(index_t reference) {
    value_t result;
    result.reference = reference;
    return result;
}

value_t array_value(array_t array) {
    value_t result;
    result.array = array;
    return result;
}

const char *get_string(value_t value) {
    return value.string;
}

void get_digit(mpz_t mpz, value_t value) {
    mpz_init_set(mpz, value.digit);
}

int64_t get_int(value_t value) {
    return mpz_get_si(value.digit);
}

bool get_bool(value_t value) {
    return value.boolean;
}

index_t get_reference(value_t value) {
    return value.reference;
}

array_t get_array(value_t value) {
    return value.array;
}
