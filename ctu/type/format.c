#include "type.h"

#include "ctu/util/str.h"

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
    case TY_LITERAL_INTEGER:
        result = ctu_strdup("int-literal");
        break;
    case TY_ANY:
        result = ctu_strdup("any");
        break;
    case TY_INTEGER: 
        result = digit_format(type->digit); 
        break;
    case TY_VOID: 
        result = ctu_strdup("void");
        break;
    case TY_CLOSURE: 
        result = closure_format(type);
        break;
    case TY_BOOL:
        result = ctu_strdup("bool");
        break;
    case TY_PTR:
        result = format("ptr(%s)", type_format(type->ptr));
        break;
    case TY_STRING:
        result = ctu_strdup("string");
        break;
    case TY_VARARGS:
        result = ctu_strdup("varargs");
        break;
    case TY_STRUCT:
        result = ctu_strdup("struct");
        break;
    case TY_UNION:
        result = ctu_strdup("union");
        break;
    case TY_POISON:
        result = format("poison(%s)", get_poison_type_message(type));
        break;
    case TY_FORWARD:
        result = format("forward(%s)", type->name);
        break;
    }

    if (!is_const(type)) {
        result = format("mut %s", result);
    }

    return result;
}
