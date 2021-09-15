#include "type.h"

#include "ctu/util/util.h"
#include "ctu/util/str.h"
#include "ctu/util/report.h"

static const char *int_format(int_t kind) {
    switch (kind) {
    case TY_CHAR: return "char";
    case TY_SHORT: return "short";
    case TY_INT: return "int";
    case TY_LONG: return "long";
    case TY_SIZE: return "size";
    case TY_INTPTR: return "intptr";
    case TY_INTMAX: return "intmax";
    default: return "";
    }
}

static char *digit_format(digit_t digit) {
    const char *ty = int_format(digit.kind);
    if (digit.sign == UNSIGNED) {
        return format("u%s", ty);
    }
    return ctu_strdup(ty);
}

static char *closure_format(const type_t *type) {
    size_t len = vector_len(type->args);
    vector_t *args = vector_of(len);
    for (size_t i = 0; i < len; i++) {
        const type_t *arg = vector_get(type->args, i);
        vector_set(args, i, type_format(arg));
    }

    char *all = strjoin(", ", args);

    char *result = type_format(type->result);

    return format("%s(%s)", result, all);
}

char *type_format(const type_t *type) {
    if (type == NULL) {
        return ctu_strdup("(null)");
    }
    
    char *result = NULL;
    switch (type->type) {
    case TY_DIGIT: 
        result = digit_format(type->digit); 
        break;
    case TY_VOID: 
        result = ctu_strdup("void");
        break;
    case TY_CLOSURE: 
        result = closure_format(type);
        break;
    default: 
        result = ctu_strdup("");
        break;
    }

    if (type->mut) {
        result = format("mut %s", result);
    }

    return result;
}

static type_t *type_new(metatype_t meta) {
    type_t *type = NEW(type_t);
    type->type = meta;
    type->mut = false;
    return type;
}

type_t *type_digit(sign_t sign, int_t kind) {
    type_t *type = type_new(TY_DIGIT);
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

bool is_digit(const type_t *type) {
    return type->type == TY_DIGIT;
}

bool is_bool(const type_t *type) {
    return type->type == TY_BOOL;
}

bool is_signed(const type_t *type) {
    if (!is_digit(type)) {
        return false;
    }

    return type->digit.sign == SIGNED;
}

bool is_unsigned(const type_t *type) {
    if (!is_digit(type)) {
        return false;
    }

    return type->digit.sign == UNSIGNED;
}

type_t *types_common(const type_t *lhs, const type_t *rhs) {
    if (is_digit(lhs) && is_digit(rhs)) {
        digit_t ld = lhs->digit;
        digit_t rd = rhs->digit;

        bool sign = ld.sign || rd.sign;
        int_t kind = MAX(ld.kind, rd.kind);

        return type_digit(sign, kind);
    }

    if (is_bool(lhs) && is_bool(rhs)) {
        return type_bool();
    }

    return type_poison("cannot get common type of unrelated types");
}
