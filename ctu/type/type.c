#include "type.h"

#include "ctu/util/util.h"
#include "ctu/util/str.h"

static type_t *type_new(metatype_t meta) {
    type_t *type = ctu_malloc(sizeof(type_t));
    type->type = meta;
    return type;
}

type_t *type_digit(bool sign, int_t kind) {
    type_t *type = type_new(TY_DIGIT);
    digit_t digit = { sign, kind };
    type->digit = digit;
    return type;
}

static const char *int_format(int_t kind) {
    switch (kind) {
    case TY_CHAR: return "char";
    case TY_SHORT: return "short";
    case TY_INT: return "int";
    case TY_LONG: return "long";
    default: return "";
    }
}

static char *digit_format(digit_t digit) {
    const char *ty = int_format(digit.kind);
    if (!digit.sign) {
        return format("u%s", ty);
    }
    return ctu_strdup(ty);
}

char *type_format(type_t *type) {
    switch (type->type) {
    case TY_DIGIT: return digit_format(type->digit);
    case TY_VOID: return ctu_strdup("void");
    default: return ctu_strdup("");
    }
}
