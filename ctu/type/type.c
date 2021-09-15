#include "type.h"

#include "ctu/util/util.h"
#include "ctu/util/report.h"

static type_t *type_new(metatype_t meta) {
    type_t *type = NEW(type_t);
    type->type = meta;
    type->mut = false;
    return type;
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

type_t *type_ptr(type_t *to) {
    type_t *type = type_new(TY_PTR);
    type->ptr = to;
    return type;
}

type_t *type_bool(void) {
    return type_new(TY_BOOL);
}

type_t *type_poison(const char *msg) {
    type_t *type = type_new(TY_POISON);
    type->msg = msg;
    return type;
}

void type_mut(type_t *type, bool mut) {
    type->mut = mut;
}
