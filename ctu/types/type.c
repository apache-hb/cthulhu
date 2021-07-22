#include "type.h"

#include "ctu/ast/ast.h"
#include "ctu/ir/ir.h"

#include "ctu/util/report.h"
#include "ctu/util/util.h"

#include "ctu/debug/type.h"
#include "ctu/debug/ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static type_t *INT_TYPES[INTEGER_END];
static type_t *UINT_TYPES[INTEGER_END];

type_t *STRING_TYPE = NULL;
type_t *BOOL_TYPE = NULL;
type_t *VOID_TYPE = NULL;

static void add_int(int kind, const char *name) {
    INT_TYPES[kind] = new_integer(kind, true, name);
}

static void add_uint(int kind, const char *name) {
    UINT_TYPES[kind] = new_integer(kind, false, name);
}

type_t *get_int_type(bool sign, integer_t kind) {
    return sign ? INT_TYPES[kind] : UINT_TYPES[kind];
}

void types_init(void) {
    add_int(INTEGER_CHAR, "char");
    add_int(INTEGER_SHORT, "short");
    add_int(INTEGER_INT, "int");
    add_int(INTEGER_LONG, "long");
    add_int(INTEGER_SIZE, "isize");
    add_int(INTEGER_INTPTR, "intptr");
    add_int(INTEGER_INTMAX, "intmax");
    
    add_uint(INTEGER_CHAR, "uchar");
    add_uint(INTEGER_SHORT, "ushort");
    add_uint(INTEGER_INT, "uint");
    add_uint(INTEGER_LONG, "ulong");
    add_uint(INTEGER_SIZE, "usize");
    add_uint(INTEGER_INTPTR, "uintptr");
    add_uint(INTEGER_INTMAX, "uintmax");
    
    STRING_TYPE = new_builtin(TYPE_STRING, "str");
    BOOL_TYPE = new_builtin(TYPE_BOOLEAN, "bool");
    VOID_TYPE = new_builtin(TYPE_VOID, "void");
}

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

static void sanitize_signed(scanner_t *source, where_t where, integer_t kind, mpz_t it) {
    switch (kind) {
    case INTEGER_CHAR:
        if (mpz_cmp_si(it, -128) < 0 || mpz_cmp_si(it, 127) > 0) {
            report(LEVEL_WARNING, source, where, "character out of range");
        }
        break;
    case INTEGER_SHORT:
        if (mpz_cmp_si(it, -32768) < 0 || mpz_cmp_si(it, 32767) > 0) {
            report(LEVEL_WARNING, source, where, "short out of range");
        }
        break;
    case INTEGER_INT:
        if (mpz_cmp_si(it, -2147483648) < 0 || mpz_cmp_si(it, 2147483647) > 0) {
            report(LEVEL_WARNING, source, where, "int out of range");
        }
        break;
    case INTEGER_LONG:
        if (mpz_cmp_si(it, -9223372036854775807LL) < 0 || mpz_cmp_si(it, 9223372036854775807LL) > 0) {
            report(LEVEL_WARNING, source, where, "long out of range");
        }
        break;

    default: 
        /**
         * all other types have platform defined ranges
         * so dont warn if they overflow
         */
        break;
    }
}

static void sanitize_unsigned(scanner_t *source, where_t where, integer_t kind, mpz_t it) {
    switch (kind) {
    case INTEGER_CHAR: 
        if (mpz_cmp_ui(it, 0xFF) > 0) {
            report(LEVEL_WARNING, source, where, "unsigned char overflow");
        }
        break;
    case INTEGER_SHORT:
        if (mpz_cmp_ui(it, 0xFFFF) > 0) {
            report(LEVEL_WARNING, source, where, "unsigned short overflow");
        }
        break;
    case INTEGER_INT:
        if (mpz_cmp_ui(it, 0xFFFFFFFF) > 0) {
            report(LEVEL_WARNING, source, where, "unsigned int overflow");
        }
        break;
    case INTEGER_LONG:
        if (mpz_cmp_ui(it, 0xFFFFFFFFFFFFFFFFULL) > 0) {
            report(LEVEL_WARNING, source, where, "unsigned long overflow");
        }
        break;

    default: 
        /**
         * same as above
         */
        break;
    }
}

void sanitize_range(type_t *type, mpz_t it, scanner_t *scanner, where_t where) {
    if (!is_integer(type)) {
        return;
    }

    if (is_signed(type)) {
        sanitize_signed(scanner, where, get_integer_kind(type), it);
    } else {
        sanitize_unsigned(scanner, where, get_integer_kind(type), it);
    }
}
