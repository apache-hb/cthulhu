#include "type.h"

#include <string.h>

#include "cthulhu/util/util.h"
#include "cthulhu/util/report.h"

static type_t *new_detailed_type(metatype_t meta, const char *name, const node_t *node) {
    type_t *type = ctu_malloc(sizeof(type_t));
    type->type = meta;
    type->mut = true;
    type->name = name;
    type->node = node;
    return type;
}

static type_t *type_new(metatype_t meta) {
    return new_detailed_type(meta, NULL, NULL);
}

aggregate_field_t *new_aggregate_field(const char *name, type_t *type) {
    aggregate_field_t *field = ctu_malloc(sizeof(aggregate_field_t)); 
    field->name = name;
    field->type = type;
    return field;
}

type_t *type_literal_integer(void) {
    return type_new(TY_LITERAL_INTEGER);
}

type_t *type_any(void) {
    return type_new(TY_ANY);
}

type_t *type_digit_with_name(const char *name, sign_t sign, int_t kind) {
    type_t *type = new_detailed_type(TY_INTEGER, name, NULL);
    digit_t digit = { sign, kind };
    type->digit = digit;
    return type;
}

type_t *type_digit(sign_t sign, int_t kind) {
    return type_digit_with_name(NULL, sign, kind);
}

type_t *type_usize(void) {
    return type_digit(UNSIGNED, TY_SIZE);
}

type_t *type_void(void) {
    return type_new(TY_VOID);
}

type_t *type_closure(vector_t *args, type_t *result) {
    type_t *type = type_new(TY_CLOSURE);
    type->args = args;
    type->result = result;
    return type;
}

type_t *type_ptr_with_index(const type_t *ptr, bool index) {
    type_t *type = type_new(TY_PTR);
    type->ptr = ptr;
    type->index = index;
    return type;
}

type_t *type_ptr(const type_t *to) {
    return type_ptr_with_index(to, false);
}

type_t *type_array(const type_t *element, size_t len) {
    type_t *type = type_new(TY_ARRAY);
    type->elements = element;
    type->len = len;
    return type;
}

type_t *type_string_with_name(const char *name) {
    return new_detailed_type(TY_STRING, name, NULL);
}

type_t *type_string(void) {
    return type_string_with_name(NULL);
}

type_t *type_bool_with_name(const char *name) {
    return new_detailed_type(TY_BOOL, name, NULL);
}

type_t *type_bool(void) {
    return type_bool_with_name(NULL);
}

type_t *type_varargs(void) {
    return type_new(TY_VARARGS);
}

type_t *type_poison_with_node(const char *msg, const node_t *node) {
    return new_detailed_type(TY_POISON, msg, node);
}

type_t *type_poison(const char *msg) {
    return type_poison_with_node(msg, NULL);
}

static type_t *new_aggregate_type(const char *name, const node_t *node, metatype_t meta, vector_t *fields) {
    type_t *type = new_detailed_type(meta, name, node);
    type->fields = fields;
    return type;
}

type_t *type_forward(const char *name, const node_t *node, void *data) {
    type_t *type = new_detailed_type(TY_FORWARD, name, node);
    type->data = data;
    return type;
}

type_t *type_struct(const char *name, const node_t *node, vector_t *fields) {
    return new_aggregate_type(name, node, TY_STRUCT, fields);
}

type_t *type_union(const char *name, const node_t *node, vector_t *fields) {
    return new_aggregate_type(name, node, TY_UNION, fields);
}

type_t *type_mut(const type_t *type, bool mut) {
    type_t *it = ctu_malloc(sizeof(type_t));
    memcpy(it, type, sizeof(type_t));
    it->mut = mut;

    if (is_array(it)) {
        it->elements = type_mut(it->elements, mut);
    }
    
    return it;
}
