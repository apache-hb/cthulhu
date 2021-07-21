#include "type.h"

#include "ast.h"

#include "ctu/util/report.h"
#include "ctu/util/util.h"

#include "ctu/debug/type.h"
#include "ctu/debug/ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

types_t *new_typelist(size_t size) {
    types_t *list = ctu_malloc(sizeof(types_t));
    list->data = ctu_malloc(sizeof(type_t*) * size);
    list->size = size;
    return list;
}

size_t typelist_len(types_t *list) {
    return list->size;
}

type_t *typelist_at(types_t *list, size_t idx) {
    return list->data[idx];
}

void typelist_put(types_t *list, size_t idx, type_t *type) {
    list->data[idx] = type;
}

static type_t *new_type(typeof_t kind, node_t *node) {
    type_t *type = ctu_malloc(sizeof(type_t));
    type->kind = kind;
    type->node = node;
    type->mut = false;
    type->invalid = false;
    type->index = SIZE_MAX;

    node->typeof = type;
    return type;
}

type_t *new_unresolved(node_t *symbol) {
    return new_type(TYPE_UNRESOLVED, symbol);
}

type_t *new_integer(integer_t kind, bool sign, const char *name) {
    type_t *type = new_type(TYPE_INTEGER, ast_type(name));
    type->sign = sign;
    type->integer = kind;
    return type;
}

type_t *new_builtin(typeof_t kind, const char *name) {
    return new_type(kind, ast_type(name));
}

type_t *new_poison(node_t *parent, const char *err) {
    type_t *type = new_type(TYPE_POISON, parent);
    type->text = err;
    return type;
}

type_t *new_callable(node_t *func, types_t *args, type_t *result) {
    type_t *type = new_type(TYPE_CALLABLE, func);
    type->args = args;
    type->result = result;
    type->function = func;
    return type;
}

type_t *new_pointer(struct node_t *node, type_t *to) {
    type_t *type = new_type(TYPE_POINTER, node);
    type->ptr = to;
    return type;
}

type_t *new_record(struct node_t *decl, const char *name) {
    type_t *type = new_type(TYPE_STRUCT, decl);

    type->name = name;

    return type;
}

void resize_record(type_t *type, size_t size) {
    type->fields.size = size;
    type->fields.fields = ctu_malloc(sizeof(field_t) * size);
}

bool is_unresolved(type_t *type) {
    return type->kind == TYPE_UNRESOLVED;
}

bool is_integer(type_t *type) {
    return type->kind == TYPE_INTEGER;
}

bool is_boolean(type_t *type) {
    return type->kind == TYPE_BOOLEAN;
}

bool is_callable(type_t *type) {
    return type->kind == TYPE_CALLABLE;
}

bool is_void(type_t *type) {
    return type->kind == TYPE_VOID;
}

bool is_struct(type_t *type) {
    return type->kind == TYPE_STRUCT;
}

integer_t get_integer_kind(type_t *type) {
    ASSERT(is_integer(type))("type is not an integer");
    return type->integer;
}

static bool is_builtin(type_t *type) {
    return is_integer(type)
        || is_boolean(type)
        || is_void(type);
}

bool is_signed(type_t *type) {
    return type->sign;
}

bool is_const(type_t *type) {
    return !type->mut;
}

bool is_pointer(type_t *type) {
    return type->kind == TYPE_POINTER;
}

type_t *set_mut(type_t *type, bool mut) {
    if (type->mut == mut) {
        return type;
    }

    type_t *out = ctu_malloc(sizeof(type_t));
    memcpy(out, type, sizeof(type_t));
    out->mut = mut;
    
    if (is_struct(out) && !type->invalid) {
        for (size_t i = 0; i < out->fields.size; i++) {
            field_t *field = &out->fields.fields[i];
            field->type = set_mut(field->type, mut);
        }
    }

    return out;
}

void connect_type(node_t *node, type_t *type) {
    node->typeof = type;

    if (!is_builtin(type) && type->node == NULL) {
        type->node = node;
    }
}
