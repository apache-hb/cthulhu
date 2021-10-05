#include "type.h"

#include "ctu/util/util.h"
#include "ctu/util/report.h"

static type_t *type_new(metatype_t meta) {
    type_t *type = ctu_malloc(sizeof(type_t));
    type->type = meta;
    type->mut = false;
    return type;
}

type_t *type_literal_integer(void) {
    return type_new(TY_LITERAL_INTEGER);
}

static type_t ANY = { 
    .type = TY_ANY, 
    .mut = true
};

const type_t *type_any(void) {
    return &ANY;
}

type_t *type_digit(sign_t sign, int_t kind) {
    type_t *type = type_new(TY_INTEGER);
    digit_t digit = { sign, kind };
    type->digit = digit;
    return type;
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

type_t *type_ptr(const type_t *to) {
    type_t *type = type_new(TY_PTR);
    type->ptr = to;
    return type;
}

type_t *type_string(void) {
    return type_new(TY_STRING);
}

type_t *type_bool(void) {
    return type_new(TY_BOOL);
}

type_t *type_varargs(void) {
    return type_new(TY_VARARGS);
}

type_t *type_poison(const char *msg) {
    type_t *type = type_new(TY_POISON);
    type->msg = msg;
    return type;
}

void type_mut(type_t *type, bool mut) {
    type->mut = mut;
}
