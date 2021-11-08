#include "value.h"
#include "module.h"

#include "ctu/util/str.h"


value_t *value_of(const type_t *type, const node_t *node) {
    value_t *value = ctu_malloc(sizeof(value_t));
    value->type = type;
    value->node = node;
    return value;
}

value_t *value_poison_with_node(const char *msg, const node_t *node) {
    return value_of(type_poison_with_node(msg, node), node);
}

value_t *value_poison(const char *msg) {
    return value_poison_with_node(msg, NULL);
}

value_t *value_bool(const node_t *node, bool value) {
    value_t *result = value_of(type_bool(), node);
    result->boolean = value;
    return result;
}

value_t *value_digit(const node_t *node, const type_t *type, mpz_t digit) {
    value_t *value = value_of(type, node);
    mpz_init_set(value->digit, digit);
    return value;
}

value_t *value_int(const node_t *node, const type_t *type, int digit) {
    value_t *value = value_of(type, node);
    mpz_init_set_si(value->digit, digit);
    return value;
}

value_t *value_ptr(const type_t *type, value_t *ptr) {
    value_t *value = value_of(type, NULL);
    value->ptr = ptr;
    return value;
}

value_t *value_block(struct block_t *block) {
    value_t *value = value_of(block->type, block->node);
    value->block = block;
    return value;
}

value_t *value_offset(const type_t *type, vector_t *base, size_t offset) {
    value_t *value = value_of(type, NULL);
    value->elements = base;
    value->offset = offset;
    return value;
}

value_t *value_vector(const type_t *type, vector_t *elements) {
    return value_offset(type, elements, 0);
}

value_t *value_empty(void) {
    return value_of(type_void(), NULL);
}

value_t *value_array(const type_t *type, size_t len) {
    vector_t *init = vector_of(len);
    for (size_t i = 0; i < len; i++) {
        vector_set(init, i, value_empty());
    }
    
    return value_vector(type, init);
}

char *value_format(const value_t *value) {
    const type_t *type = value->type;

    if (is_void(type)) {
        return ctu_strdup("empty");
    }

    char *typestr = type_format(type);

    if (is_digit(type)) {
        char *str = mpz_get_str(NULL, 10, value->digit);
        return format("%s(%s)", typestr, str);
    }

    if (is_poison(type)) {
        return typestr;
    }

    if (is_closure(type)) {
        return format("%s(&%s)", typestr, value->block->name);
    }

    if (is_array(type)) {
        size_t len = static_array_length(type);
        vector_t *parts = vector_of(len);
        for (size_t i = 0; i < len; i++) {
            value_t *elem = vector_get(value->elements, i);
            char *fmt = value_format(elem);
            vector_set(parts, i, fmt);
        }
        char *body = strjoin(", ", parts);
        return format("%s(%zu[%s])", typestr, len, body);
    }

    /* trigraphs are fun */
    return format("%s(\?\?\?)", typestr);
}
