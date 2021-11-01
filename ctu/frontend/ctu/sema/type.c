#include "type.h"

void forward_type(sema_t *sema, const char *name, ctu_t *ctu) {
    type_t *type = get_type(sema, name);
    if (type != NULL) {
        message_t *id = report(sema->reports, ERROR, ctu->node, "type `%s` already defined", name);
        if (type->node == NULL) {
            report_note(id, "`%s` is a builtin type", type->name);
        } else {
            report_append(id, type->node, "previously declared here");
        }
        return;
    }

    type_t *fwd = type_forward(name, ctu->node, ctu);
    add_type(sema, name, fwd);
}

static void build_newtype(sema_t *sema, type_t *type, ctu_t *ctu) {
    *type = *compile_type(sema, ctu->result);
}

void build_type(sema_t *sema, type_t *type) {
    ctu_t *ctu = type->data;

    switch (ctu->type) {
    case CTU_NEWTYPE: 
        build_newtype(sema, type, ctu);
        break;

    default:
        ctu_assert(sema->reports, "(ctu) unimplemented build-type %d", ctu->type);
        break;
    }
}

static type_t *compile_typepath(sema_t *sema, ctu_t *ctu) {
    size_t idx = 0;
    size_t len = vector_len(ctu->path);
    while (idx < len - 1) {
        const char *name = vector_get(ctu->path, idx++);
        sema_t *nest = get_module(sema, name);
        if (nest == NULL) {
            report(sema->reports, ERROR, ctu->node, "failed to resolve path segment `%s`", name);
            return type_poison_with_node("unresolved type", ctu->node);
        }
    }

    type_t *type = get_type(sema, vector_tail(ctu->path));
    if (type == NULL) {
        report(sema->reports, ERROR, ctu->node, "unable to resolve type name `%s`", ctu->ident);
        return type_poison_with_node("unresolved type", ctu->node);
    }
    return type;
}

static type_t *compile_pointer(sema_t *sema, ctu_t *ctu) {
    type_t *type = compile_type(sema, ctu->ptr);
    return type_ptr_with_index(type, ctu->subscript);
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

static type_t *compile_array(sema_t *sema, ctu_t *ctu) {
    UNUSED(sema);
    UNUSED(ctu);

    return type_poison("compile-array unimplemented");
}

type_t *compile_type(sema_t *sema, ctu_t *ctu) {
    switch (ctu->type) {
    case CTU_TYPEPATH:
        return compile_typepath(sema, ctu);
    case CTU_POINTER:
        return compile_pointer(sema, ctu);
    case CTU_CLOSURE:
        return compile_closure(sema, ctu);
    case CTU_MUTABLE:
        return compile_mutable(sema, ctu);
    case CTU_VARARGS:
        return type_varargs();
    case CTU_ARRAY:
        return compile_array(sema, ctu);

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

    if (is_bool(lhs) && is_bool(rhs)) {
        return type_bool();
    }

    return type_poison("cannot find common type");
}

static lir_t *convert_expr(sema_t *sema, lir_t *expr, const type_t *type, bool implicit) {
    const type_t *exprtype = lir_type(expr);

    /**
     * casting from a digit to another digit
     */
    if (is_digit(type) && is_digit(exprtype)) {
        if (implicit) {
            if (type->digit.kind < exprtype->digit.kind) {
                report(sema->reports, WARNING, expr->node, "implicit truncation from `%s` to `%s`", type_format(exprtype), type_format(type));
            }

            if (type->digit.sign != exprtype->digit.sign) {
                report(sema->reports, WARNING, expr->node, "implicit sign conversion from `%s` to `%s`", type_format(exprtype), type_format(type));
            }
        }

        return lir_cast(expr->node, type, expr);
    }

    /**
     * casting from a digit to a bool becomes `expr != 0`
     */
    if (is_bool(type) && is_digit(exprtype)) {
        lir_t *zero = lir_int(expr->node, type_digit(SIGNED, TY_INT), 0);
        return lir_binary(expr->node, type_bool(), BINARY_NEQ, expr, zero);
    }

    if (implicit) {
        if (is_pointer(type) && is_pointer(exprtype)) {
            if (is_voidptr(type) || is_voidptr(exprtype)) {
                return expr;
            }

            report(sema->reports, ERROR, expr->node, "implicit conversion from `%s` to `%s`", type_format(exprtype), type_format(type));
        }
    } else {
        if (is_pointer(type) && is_pointer(exprtype)) {
            return lir_cast(expr->node, type, expr);
        }
    }

    if (is_digit(type) && is_pointer(exprtype)) {
        digit_t digit = type->digit;
        if (digit.kind == TY_INTPTR) {
            return lir_cast(expr->node, type, expr);
        }
        message_t *id = report(sema->reports, ERROR, expr->node, "invalid cast from `%s` to `%s`", type_format(exprtype), type_format(type));
        report_note(id, "pointers can only be cast to `uintptr`");
    }

    if (is_pointer(type) && is_digit(exprtype)) {
        digit_t digit = type->digit;
        if (digit.kind == TY_INTPTR) {
            return lir_cast(expr->node, type, expr);
        }
        message_t *id = report(sema->reports, ERROR, expr->node, "invalid cast from `%s` to `%s`", type_format(exprtype), type_format(type));
        report_note(id, "only `uintptr` can be casted to a pointer");
    }

    if (is_void(type) && is_void(exprtype)) {
        return expr;
    }

    if (is_string(type) && is_string(exprtype)) {
        return expr;
    }

    if (is_bool(type) && is_bool(exprtype)) {
        return expr;
    }

    return NULL;
}

lir_t *implicit_convert_expr(sema_t *sema, lir_t *expr, const type_t *type) {
    return convert_expr(sema, expr, type, true);
}

lir_t *explicit_convert_expr(sema_t *sema, lir_t *expr, const type_t *type) {
    return convert_expr(sema, expr, type, false);
}
