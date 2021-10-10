#include "expr.h"
#include "value.h"
#include "define.h"

#include "ctu/type/retype.h"

static lir_t *compile_digit(ctu_t *digit) {
    return lir_digit(digit->node, 
        /* type = */ type_literal_integer(), 
        /* digit = */ digit->digit
    );
}

static lir_t *compile_bool(ctu_t *value) {
    return lir_bool(value->node, type_bool(), value->boolean);
}

static lir_t *compile_unary(sema_t *sema, ctu_t *expr) {
    lir_t *operand = compile_expr(sema, expr->operand);
    const type_t *type = lir_type(operand);
    unary_t unary = expr->unary;

    switch (unary) {
    case UNARY_ABS: case UNARY_NEG:
        if (!is_integer(type)) {
            report(sema->reports, ERROR, expr->node, "unary math requires an integer operand, `%s` provided", type_format(type));
        }
        break;
    case UNARY_ADDR:
        type = type_ptr(type);
        break;
    case UNARY_DEREF:
        if (!is_pointer(type)) {
            report(sema->reports, ERROR, expr->node, "unary dereference requires a pointer operand, `%s` provided", type_format(type));
        } else {
            type = type->ptr;
        }
        break;
    default:
        ctu_assert(sema->reports, "compile-unary unknown op %d", unary);
        break;
    }

    return lir_unary(expr->node, type, unary, operand);
}

static const type_t *binary_equal(reports_t *reports, node_t *node, lir_t *lhs, lir_t *rhs) {
    const type_t *type = types_common(lir_type(lhs), lir_type(rhs));
    if (is_poison(type)) {
        report(reports, ERROR, node, "cannot take equality of `%s` and `%s`",
            type_format(lir_type(lhs)),
            type_format(lir_type(rhs))
        );
    }

    return type_bool();
}

static const type_t *binary_compare(reports_t *reports, node_t *node, lir_t *lhs, lir_t *rhs) {
    const type_t *type = types_common(lir_type(lhs), lir_type(rhs));
    if (!is_integer(type)) {
        report(reports, ERROR, node, "cannot compare `%s` to `%s`", 
            type_format(lir_type(lhs)), 
            type_format(lir_type(rhs))
        );
    }

    return type_bool();
}

static const type_t *binary_logic(reports_t *reports, node_t *node, lir_t *lhs, lir_t *rhs) {
    const type_t *type = types_common(lir_type(lhs), lir_type(rhs));
    if (!is_bool(type)) {
        report(reports, ERROR, node, "cannot logically evaluate `%s` with `%s`", 
            type_format(lir_type(lhs)), 
            type_format(lir_type(rhs))
        );
    }

    return type_bool();
}

static const type_t *binary_math(reports_t *reports, node_t *node, lir_t *lhs, lir_t *rhs) {
    const type_t *type = types_common(lir_type(lhs), lir_type(rhs));
    if (!is_integer(type)) {
        report(reports, ERROR, node, "cannot perform math operations on `%s` with `%s`",
            type_format(lir_type(lhs)),
            type_format(lir_type(rhs))
        );
    }

    return type;
}

static const type_t *binary_bits(reports_t *reports, node_t *node, lir_t *lhs, lir_t *rhs) {
    const type_t *type = types_common(lir_type(lhs), lir_type(rhs));
    if (!is_integer(type)) {
        report(reports, ERROR, node, "cannot perform bitwise operations on `%s` with `%s`",
            type_format(lir_type(lhs)),
            type_format(lir_type(rhs))
        );
    }

    return type;
}

static const type_t *binary_type(reports_t *reports, node_t *node, binary_t binary, lir_t *lhs, lir_t *rhs) {
    switch (binary) {
    case BINARY_EQ: case BINARY_NEQ:
        return binary_equal(reports, node, lhs, rhs);
    
    case BINARY_GT: case BINARY_GTE:
    case BINARY_LT: case BINARY_LTE:
        return binary_compare(reports, node, lhs, rhs);

    case BINARY_AND: case BINARY_OR:
        return binary_logic(reports, node, lhs, rhs);

    case BINARY_ADD: case BINARY_SUB:
    case BINARY_MUL: case BINARY_DIV: case BINARY_REM:
        return binary_math(reports, node, lhs, rhs);

    case BINARY_SHL: case BINARY_SHR:
    case BINARY_BITAND: case BINARY_BITOR: case BINARY_XOR:
        return binary_bits(reports, node, lhs, rhs);

    default:
        ctu_assert(reports, "binary-type unreachable %d", binary);
        return type_poison("unreachable");
    }
}

static lir_t *compile_binary(sema_t *sema, ctu_t *expr) {
    lir_t *lhs = compile_expr(sema, expr->lhs);
    lir_t *rhs = compile_expr(sema, expr->rhs);
    binary_t binary = expr->binary;

    const type_t *common = binary_type(sema->reports, expr->node, binary, lhs, rhs);

    return lir_binary(expr->node, common, binary, lhs, rhs);
}

static lir_t *compile_call(sema_t *sema, ctu_t *expr) {
    size_t len = vector_len(expr->args);
    lir_t *func = compile_expr(sema, expr->func);
    vector_t *args = vector_of(len);
    for (size_t i = 0; i < len; i++) {
        ctu_t *it = vector_get(expr->args, i);
        lir_t *arg = compile_expr(sema, it);
        vector_set(args, i, arg);
    }

    const type_t *fty = lir_type(func);
    if (!is_closure(fty)) {
        report(sema->reports, ERROR, expr->node, "cannot apply call to type `%s`", type_format(fty));
        return lir_poison(expr->node, "invalid call");
    }

    size_t total = maximum_params(fty);
    if (total < len) {
        if (is_variadic(fty)) {
            size_t least = minimum_params(fty);
            report(sema->reports, ERROR, expr->node, "incorrect number of arguments, expected at least %zu have %zu", least, len);
        } else {
            report(sema->reports, ERROR, expr->node, "incorrect number of arguments, expected %zu have %zu", total, len);
        }

        return lir_poison(expr->node, "invalid call");
    }

    for (size_t i = 0; i < len; i++) {
        const type_t *want = param_at(fty, i);
        lir_t *arg = vector_get(args, i);
        lir_t *res = retype_expr(sema->reports, want, arg);

        if (is_poison(lir_type(res))) {
            report(sema->reports, ERROR, res->node, "cannot convert cast argument %zu from `%s` to `%s`", i, type_format(lir_type(arg)), type_format(want));
        }

        vector_set(args, i, res);
    }

    return lir_call(expr->node, closure_result(fty), func, args);
}

static lir_t *compile_name(sema_t *sema, ctu_t *expr) {
    const char *name = expr->ident;
    if (is_discard(name)) {
        report(sema->reports, WARNING, expr->node, "reading from a discarded identifier `%s`", name);
    }
    
    lir_t *var = get_var(sema, name);
    if (var != NULL) {
        if (lir_is(var, LIR_PARAM)) {
            return var;
        }

        lir_t *it = compile_value(var);
        return lir_name(expr->node, lir_type(it), it);
    }

    lir_t *func = get_func(sema, name);
    if (func != NULL) {
        lir_t *it = compile_define(func);
        return it;
    }

    report(sema->reports, ERROR, expr->node, "failed to resolve `%s`", name);
    return lir_poison(expr->node, "unresolved name");
}

static lir_t *compile_string(ctu_t *expr) {
    return lir_string(expr->node, type_string(), expr->str);
}

lir_t *compile_expr(sema_t *sema, ctu_t *expr) {
    switch (expr->type) {
    case CTU_DIGIT: return compile_digit(expr);
    case CTU_BOOL: return compile_bool(expr);
    case CTU_UNARY: return compile_unary(sema, expr);
    case CTU_BINARY: return compile_binary(sema, expr);
    case CTU_CALL: return compile_call(sema, expr);
    case CTU_IDENT: return compile_name(sema, expr);
    case CTU_STRING: return compile_string(expr);

    default:
        ctu_assert(sema->reports, "(ctu) compile-expr unimplemented expr type %d", expr->type);
        return lir_poison(expr->node, "unimplemented");
    }
}

lir_t *compile_stmts(sema_t *sema, ctu_t *stmts) {
    vector_t *all = stmts->stmts;
    size_t len = vector_len(all);

    vector_t *result = vector_of(len);
    for (size_t i = 0; i < len; i++) {
        ctu_t *it = vector_get(stmts->stmts, i);
        lir_t *stmt = compile_stmt(sema, it);
        vector_set(result, i, stmt);
    }

    return lir_stmts(stmts->node, result);
}

static lir_t *compile_local(sema_t *sema, ctu_t *stmt) {
    lir_t *value = local_value(sema, stmt);
    add_local(sema, value);
    add_var(sema, stmt->name, value);
    
    if (value->init) {
        return lir_assign(stmt->node, value, value->init);
    } 

    /* basically a no-op */
    return lir_stmts(stmt->node, vector_new(0));
}

static lir_t *compile_return(sema_t *sema, ctu_t *stmt) {
    lir_t *operand = NULL;
    if (stmt->operand != NULL) {
        lir_t *lir = compile_expr(sema, stmt->operand);
        operand = retype_expr(sema->reports, get_return(sema), lir);
    }

    return lir_return(stmt->node, operand);
}

static lir_t *compile_while(sema_t *sema, ctu_t *stmt) {
    lir_t *cond = compile_expr(sema, stmt->cond);
    lir_t *body = compile_stmt(sema, stmt->then);

    return lir_while(stmt->node, 
        retype_expr(sema->reports, type_bool(), cond), 
        body
    );
}

static lir_t *compile_assign(sema_t *sema, ctu_t *stmt) {
    lir_t *dst = compile_expr(sema, stmt->dst);
    lir_t *src = compile_expr(sema, stmt->src);

    /* TODO: a touch hacky */
    if (!lir_is(dst, LIR_NAME)) {
        report(sema->reports, ERROR, stmt->node, "can only assign to named expressions");
        dst = lir_poison(stmt->node, "invalid assigment");
    } else {
        dst = dst->it;
    }

    return lir_assign(stmt->node, dst, 
        retype_expr(sema->reports, lir_type(dst), src)
    );
}

static lir_t *compile_branch(sema_t *sema, ctu_t *stmt) {
    lir_t *cond = compile_expr(sema, stmt->cond);
    lir_t *then = compile_stmt(sema, stmt->then);

    return lir_branch(stmt->node, 
        retype_expr(sema->reports, type_bool(), cond), 
        then,
        NULL
    );
}

static size_t SMALL_SIZES[TAG_MAX] = { MAP_SMALL, MAP_SMALL, MAP_SMALL };

lir_t *compile_stmt(sema_t *sema, ctu_t *stmt) {
    switch (stmt->type) {
    case CTU_STMTS: return compile_stmts(new_sema(sema->reports, sema, SMALL_SIZES), stmt);
    case CTU_RETURN: return compile_return(sema, stmt);
    case CTU_WHILE: return compile_while(sema, stmt);
    case CTU_ASSIGN: return compile_assign(sema, stmt);
    case CTU_BRANCH: return compile_branch(sema, stmt);

    case CTU_CALL: return compile_call(sema, stmt);
    case CTU_VALUE: return compile_local(sema, stmt);

    case CTU_IDENT:
        report(sema->reports, WARNING, stmt->node, "expression has no effect");
        return lir_stmts(stmt->node, vector_new(0));

    default:
        ctu_assert(sema->reports, "compile-stmt unknown type %d", stmt->type);
        return lir_poison(stmt->node, "compile-stmt unknown type");
    }
}
