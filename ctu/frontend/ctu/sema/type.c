#include "type.h"
#include "expr.h"

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

    return type_mut(type, ctu->mut);
}

static type_t *compile_array(sema_t *sema, ctu_t *ctu) {
    lir_t *size = compile_expr(sema, ctu->size);
    if (size->leaf != LIR_DIGIT) {
        ctu_assert(sema->reports, "(ctu) array size must be a literal");
        return type_poison_with_node("array size must be a literal", ctu->node);
    }

    type_t *of = compile_type(sema, ctu->arr);
    return type_array(of, mpz_get_ui(size->digit));
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

/**
 * implicit rules:
 *      var <- const 
 *          then error "casting away mutability"
 * 
 *      digit <- digit when
 *          dst.width < src.width 
 *              then error "possible truncation"
 *          dst.sign != src.sign
 *              then error "lossy sign conversion"
 *      
 *      ptr <- digit when
 *          src.width != INTPTR
 *              then error "possible lossy conversion"
 * 
 *      ptr <- closure when
 *          ptr.type != void
 *              then error "can only convert closure to void pointer"
 * 
 *      ptr <- array when
 *          ptr.type != array.type
 *              then error "can only convert arrays to pointers of the same type"
 * 
 *      digit <- ptr when
 *          dst.width != INTPTR
 *              then error "can only convert pointers to intptrs"
 * 
 *      closure <- ptr
 *          when src.type != void
 *              then error "can only convert void pointers to closures"
 * 
 *      array <- ptr
 *          when dst.type != src.type
 *              then error "can only convert pointer to array when types are the same"
 * 
 *      bool <- digit
 *          dst = src != 0
 * 
 *      
 */
static lir_t *convert_expr(sema_t *sema, lir_t *expr, const type_t *dst, bool implicit) {
    const type_t *src = lir_type(expr);
    
    if (types_exact_equal(src, dst)) {
        return expr;
    }

    if (is_array(dst) && is_array(src)) {
        if (types_exact_equal(dst->elements, src->elements)) {
            if (dst->len >= src->len) {
                return expr;
            }
            report(sema->reports, WARNING, expr->node, "array shortened from `%zu` to `%zu`", src->len, dst->len);
            return expr;
        }
        return NULL;
    }

    /* warn about mutability being cast away */
    if (implicit) {
        if (!is_const(dst) && is_const(src)) {
            report(sema->reports, WARNING, expr->node, "implicitly removing const");
        }
    }

    /* handle integer conversion */
    if (is_digit(dst) && is_digit(src)) {
        if (implicit) {
            digit_t lhs = dst->digit;
            digit_t rhs = src->digit;

            if (lhs.kind < rhs.kind) {
                report(sema->reports, WARNING, expr->node,
                    "implicit integer truncation from `%s` to `%s`",
                    ctu_type_format(src),
                    ctu_type_format(dst)
                );
            }

            if (lhs.sign != rhs.sign) {
                report(sema->reports, WARNING, expr->node,
                    "implicit sign conversion from `%s` to `%s`",
                    ctu_type_format(src),
                    ctu_type_format(dst)
                );
            }
        }

        return lir_cast(expr->node, dst, expr);
    }

    /* handle digit to bool conversion */
    if (is_bool(dst) && is_digit(src)) {
        lir_t *zero = lir_int(expr->node, src, 0);
        lir_t *cmp = lir_binary(expr->node, get_cached_bool_type(sema), BINARY_NEQ, expr, zero);
        return cmp;
    }

    if (type_is_indirect(dst) && is_voidptr(src)) {
        return lir_cast(expr->node, dst, expr);
    }

    if (is_voidptr(dst) && type_is_indirect(src)) {
        return lir_cast(expr->node, dst, expr);
    }

    /* handle basic cases */
    if (
        (is_void(dst) && is_void(src)) ||
        (is_bool(dst) && is_bool(src)) ||
        (is_string(dst) && is_string(src))
    ) {
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

static char *fmt_array(const type_t *type) {
    return format("[%s * %zu]", ctu_type_format(type->elements), type->len);
}

static char *fmt_ptr(const type_t *type) {
    if (type->index) {
        return format("[%s]", ctu_type_format(type->ptr));
    } else {
        return format("*%s", ctu_type_format(type->ptr));
    }
}

static char *fmt_closure(const type_t *type) {
    char *result = ctu_type_format(type->result);
    vector_t *args = VECTOR_MAP(type->args, ctu_type_format);
    char *joined = strjoin(", ", args);
    return format("(%s) -> %s", joined, result);
}

char *ctu_type_format(const type_t *type) {
    switch (type->type) {
    case TY_ARRAY: return fmt_array(type);
    case TY_PTR: return fmt_ptr(type);
    case TY_CLOSURE: return fmt_closure(type);
    default: return type->name != NULL ? ctu_strdup(type->name) : NULL;
    }
}
