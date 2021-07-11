#include "type.h"

#include <stdio.h>

static const char *integer_name(type_t *type) {
    bool sign = is_signed(type);
    switch (type->integer) {
    case INTEGER_CHAR: return sign ? "char" : "uchar";
    case INTEGER_SHORT: return sign ? "short" : "ushort";
    case INTEGER_INT: return sign ? "int" : "uint";
    case INTEGER_LONG: return sign ? "long" : "ulong";
    case INTEGER_SIZE: return sign ? "isize" : "usize";
    case INTEGER_INTPTR: return sign ? "intptr" : "uintptr";
    case INTEGER_INTMAX: return sign ? "intmax" : "uintmax";
    default: return "unknown";
    }
}

void debug_type(type_t *type);

static void debug_callable(type_t *type) {
    types_t *args = type->args;
    printf("(");
    for (size_t i = 0; i < typelist_len(args); i++) {
        if (i != 0) {
            printf(", ");
        }
        debug_type(typelist_at(args, i));
    }
    printf(") -> ");
    debug_type(type->result);
}

void debug_type(type_t *type) {
    switch (type->kind) {
    case TYPE_INTEGER: printf("%s", integer_name(type)); break;
    case TYPE_BOOLEAN: printf("bool"); break;
    case TYPE_VOID: printf("void"); break;
    case TYPE_CALLABLE: debug_callable(type); break;
    case TYPE_POINTER: printf("*"); debug_type(type->ptr); break;
    case TYPE_POISON: printf("poison(%s)", type->text); break;
    case TYPE_UNRESOLVED: printf("unresolved"); break;
    }
}
