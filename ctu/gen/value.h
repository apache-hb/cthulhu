#pragma once

#include "ctu/type/type.h"
#include "ctu/ast/ast.h"

#include <gmp.h>

typedef struct value_t {
    const type_t *type;
    const node_t *node;

    struct {
        struct {
            vector_t *elements;
            size_t offset;
        };

        mpz_t digit;
        bool boolean;
        const char *string;
        struct value_t *ptr;
        struct block_t *block;
    };
} value_t;

value_t *value_of(const type_t *type, const node_t *node);
value_t *value_poison_with_node(const char *msg, const node_t *node);
value_t *value_poison(const char *msg);
value_t *value_bool(const node_t *node, bool value);
value_t *value_digit(const node_t *node, const type_t *type, mpz_t digit);
value_t *value_int(const node_t *node, const type_t *type, int digit);
value_t *value_ptr(const type_t *type, value_t *ptr);
value_t *value_block(struct block_t *block);
value_t *value_vector(const type_t *type, vector_t *elements);
value_t *value_offset(const type_t *type, vector_t *base, size_t offset);
value_t *value_array(const type_t *type, size_t len);
value_t *value_empty(void);

char *value_format(const value_t *value);

void value_delete(value_t *value);
