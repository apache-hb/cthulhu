#include "type.h"

static type_t *compile_typename(sema_t *sema, ctu_t *ctu) {
    type_t *type = get_type(sema, ctu->ident);
    if (type == NULL) {
        report(sema->reports, ERROR, ctu->node, "unable to resolve type name `%s`", ctu->ident);
        return type_poison_with_node("unresolved type", ctu->node);
    }
    return type;
}

static type_t *compile_pointer(sema_t *sema, ctu_t *ctu) {
    type_t *type = compile_type(sema, ctu->ptr);
    return type_ptr(type);
}

static type_t *compile_closure(sema_t *sema, ctu_t *ctu) {
    type_t *result = compile_type(sema, ctu->result);
    
    size_t len = vector_len(ctu->params);
    vector_t *args = vector_of(len);

    for (size_t i = 0; i < len; i++) {
        ctu_t *param = vector_get(ctu->params, i);
        type_t *type = compile_type(sema, param);
        vector_set(args, i, type);
    }

    return type_closure(args, result);
}

static type_t *compile_mutable(sema_t *sema, ctu_t *ctu) {
    type_t *type = compile_type(sema, ctu->kind);

    if (!is_const(type)) {
        message_t *id = report(sema->reports, WARNING, ctu->node, "type is already mutable");
        report_underline(id, "redundant mutable specifier");
    }

    return type_mut(type, true);
}

type_t *compile_type(sema_t *sema, ctu_t *ctu) {
    switch (ctu->type) {
    case CTU_TYPENAME:
        return compile_typename(sema, ctu);
    case CTU_POINTER:
        return compile_pointer(sema, ctu);
    case CTU_CLOSURE:
        return compile_closure(sema, ctu);
    case CTU_MUTABLE:
        return compile_mutable(sema, ctu);

    default:
        ctu_assert(sema->reports, "compile-type unknown type %d", ctu->type);
        return type_poison_with_node("unknown type", ctu->node);
    }
}

type_t *common_digit(const type_t *lhs, const type_t *rhs) {
    digit_t left = lhs->digit;
    digit_t right = rhs->digit;

    sign_t sign = (left.sign == SIGNED || right.sign == SIGNED) ? SIGNED : UNSIGNED;
    int_t width = MAX(left.kind, right.kind);

    return type_digit(sign, width);
}

type_t *next_digit(type_t *type) {
    if (!is_digit(type)) {
        return type;
    }

    digit_t digit = type->digit;
    if (digit.kind == TY_INTMAX) {
        return type;
    }

    return type_digit(digit.sign, digit.kind + 1);
}

type_t *common_type(const type_t *lhs, const type_t *rhs) {
    if (is_digit(lhs) && is_digit(rhs)) {
        return common_digit(lhs, rhs);
    }

    return type_poison("cannot find common type");
}
