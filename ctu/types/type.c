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

static size_t index = 0;

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

static type_t *new_type(typeof_t kind, node_t *node) {
    type_t *type = ctu_malloc(sizeof(type_t));
    type->kind = kind;
    type->node = node;
    type->mut = false;
    type->lvalue = false;
    type->index = index++;

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

type_t *new_callable(node_t *func, list_t *args, type_t *result) {
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

type_t *new_struct(struct node_t *decl, const char *name) {
    type_t *type = new_type(TYPE_STRUCT, decl);

    type->name = name;

    return type;
}

void resize_struct(type_t *type, size_t size) {
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
    return !type->lvalue;
}

bool is_pointer(type_t *type) {
    return type->kind == TYPE_POINTER;
}

type_t *copyty(type_t *type) {
    type_t *copy = ctu_malloc(sizeof(type_t));
    memcpy(copy, type, sizeof(type_t));
    return copy;
}

type_t *set_mut(type_t *type, bool mut) {
    if (type->mut == mut) {
        return type;
    }

    type_t *copy = copyty(type);
    copy->mut = mut;

    return copy;
}

type_t *set_lvalue(type_t *type, bool lvalue) {
    if (type->lvalue == lvalue) {
        return type;
    }

    type_t *out = copyty(type);
    out->lvalue = lvalue;

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

static node_t *implicit_cast(node_t *original, type_t *to) {
    node_t *cast = ast_cast(original->scanner, original->where, original, NULL);

    connect_type(cast, to);

    return make_implicit(cast);
}

static bool type_can_become(node_t **node, type_t *dst, type_t *src, bool implicit) {
    if (is_integer(dst) && is_integer(src)) {
        integer_t to = get_integer_kind(dst);
        integer_t from = get_integer_kind(src);

        if ((from > to) && implicit) {
            reportf(LEVEL_WARNING, *node, "implcit narrowing cast to a smaller integer type");
            *node = implicit_cast(*node, dst);
        }

        if ((dst->sign != src->sign) && implicit) {
            reportf(LEVEL_WARNING, *node, "implicit sign change");
            *node = implicit_cast(*node, dst);
        }

        return true;
    }

    if (is_pointer(dst) && is_pointer(src)) {
        /* allow explicit casts between any pointer types */
        if (!implicit) {
            return true;
        }

        if (dst->mut && !src->mut) {
            reportf(LEVEL_ERROR, *node, "cannot implicitly remove const");
            return false;
        }

        return type_can_become(node, dst->ptr, src->ptr, implicit);
    }

    if (is_struct(src) && is_struct(dst)) {
        if (src->index != dst->index) {
            reportf(LEVEL_ERROR, *node, "cannot implicitly cast between unrelated types");
            return false;
        }
    }

    if (is_integer(dst) && is_pointer(src)) {
        if (get_integer_kind(dst) != INTEGER_INTPTR) {
            reportf(LEVEL_ERROR, *node, "cannot implicitly cast pointer to a non intptr integer");
            return false;
        } else {
            return true;
        }
    }

    /**
     * implicit boolean conversion
     */
    if (is_boolean(dst) && is_integer(src)) {
        if (implicit) {
            reportf(LEVEL_WARNING, *node, "implicit cast to boolean");
        }
        *node = implicit_cast(*node, dst);
        return true;
    }

    return dst->index == src->index;
}

bool types_equal(type_t *type, type_t *other) {
    return type->index == other->index;
}

bool type_can_become_implicit(node_t **node, type_t *dst, type_t *src) {
    return type_can_become(node, dst, src, true);
}

bool type_can_become_explicit(node_t **node, type_t *dst, type_t *src) {
    return type_can_become(node, dst, src, false);
}
