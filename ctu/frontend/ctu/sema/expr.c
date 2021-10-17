#include "expr.h"
#include "type.h"
#include "value.h"
#include "define.h"

static lir_t *compile_lvalue(sema_t *sema, ctu_t *expr);

static lir_t *compile_digit(ctu_t *digit) {
    return lir_digit(digit->node, 
        /* type = */ type_digit(SIGNED, TY_INT), 
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

/**
 * handle `==` and `!=` expressions
 */
static const type_t *binary_equal(reports_t *reports, node_t *node, lir_t *lhs, lir_t *rhs) {
    const type_t *type = common_type(lir_type(lhs), lir_type(rhs));
    if (is_poison(type)) {
        report(reports, ERROR, node, "cannot take equality of `%s` and `%s`",
            type_format(lir_type(lhs)),
            type_format(lir_type(rhs))
        );
    }

    return type_bool();
}

/**
 * handle `<`, `<=`, `>`, and `>=` expressions
 */
static const type_t *binary_compare(reports_t *reports, node_t *node, lir_t *lhs, lir_t *rhs) {
    const type_t *type = common_type(lir_type(lhs), lir_type(rhs));
    if (!is_integer(type)) {
        report(reports, ERROR, node, "cannot compare `%s` to `%s`", 
            type_format(lir_type(lhs)), 
            type_format(lir_type(rhs))
        );
    }

    return type_bool();
}

/**
 * handle `&&` and `||` expressions
 */
static const type_t *binary_logic(reports_t *reports, node_t *node, lir_t *lhs, lir_t *rhs) {
    const type_t *type = common_type(lir_type(lhs), lir_type(rhs));
    if (!is_bool(type)) {
        report(reports, ERROR, node, "cannot logically evaluate `%s` with `%s`", 
            type_format(lir_type(lhs)), 
            type_format(lir_type(rhs))
        );
    }

    return type_bool();
}

/**
 * handle `+`, `-`, `*`, and `/` expressions
 */
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
        return type_poison_with_node("unreachable", node);
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
        lir_t *arg = vector_get(args, i);
        const type_t *want = param_at(fty, i);

        if (is_poison(lir_type(arg))) {
            report(sema->reports, ERROR, arg->node, "cannot convert cast argument %zu from `%s` to `%s`", i, type_format(lir_type(arg)), type_format(want));
        }

        vector_set(args, i, arg);
    }

    return lir_call(expr->node, closure_result(fty), func, args);
}

static lir_t *compile_name(sema_t *sema, ctu_t *expr) {
    const char *name = expr->ident;
    if (is_discard(name)) {
        report(sema->reports, ERROR, expr->node, "reading from a discarded identifier `%s`", name);
    }
    
    lir_t *var = get_var(sema, name);
    if (var != NULL) {
        if (lir_is(var, LIR_PARAM)) {
            return var;
        }

        return compile_value(var);
    }

    lir_t *func = get_func(sema, name);
    if (func != NULL) {
        lir_t *it = compile_define(func);
        return it;
    }

    report(sema->reports, ERROR, expr->node, "failed to resolve `%s`", name);
    return lir_poison(expr->node, "unresolved name");
}

static lir_t *compile_access(sema_t *sema, ctu_t *expr) {
    lir_t *obj = compile_expr(sema, expr->object);
    const type_t *type = lir_type(obj);
    const type_t *aggregate = is_pointer(type) ? type->ptr : type;
    
    bool error = false;

    if (is_pointer(type) && !expr->indirect) {
        message_t *id = report(sema->reports, ERROR, expr->node, "cannot directly access pointer type `%s`", type_format(type));
        report_underline(id, "use `->` for accessing pointers to aggregates");
        error = true;
    } else if (!is_pointer(type) && expr->indirect) {
        message_t *id = report(sema->reports, ERROR, expr->node, "cannot indirectly access non-pointer type `%s`", type_format(type));
        report_underline(id, "use `.` for accessing aggregates");
        error = true;
    }

    if (!is_aggregate(aggregate) && !error) {
        message_t *id = report(sema->reports, ERROR, expr->node, "only aggregate types can be accessed");
        report_underline(id, "`%s` is not an aggregate type", type_format(type));
        report_note(id, "user defined structs and unions are aggregate types");
        error = true;
    }

    size_t field_index = field_offset(type, expr->field);
    if (field_index == SIZE_MAX && !error) {
        report(sema->reports, ERROR, expr->node, "type `%s` has no field named `%s`", type_format(aggregate), expr->field);
    }

    const type_t *field_type = get_field(aggregate, field_index);

    return lir_access(expr->node, field_type, obj, field_index);
}

static lir_t *compile_string(ctu_t *expr) {
    return lir_string(expr->node, type_string(), expr->str);
}

static lir_t *name_expr(node_t *node, lir_t *lir) {
    return lir_name(node, lir_type(lir), lir);
}

static lir_t *compile_lvalue(sema_t *sema, ctu_t *expr) {
    switch (expr->type) {
    case CTU_UNARY:
        /* hacky but pretty at the same time */
        if (expr->unary == UNARY_DEREF) {
            return compile_unary(sema, expr);
        }
        // fallthrough
    case CTU_DIGIT: case CTU_BOOL: case CTU_BINARY:
    case CTU_CALL: case CTU_STRING:
        report(sema->reports, ERROR, expr->node, "expression is not an lvalue");
        return lir_poison(expr->node, "malformed lvalue");

    case CTU_IDENT: return compile_name(sema, expr);
    case CTU_ACCESS: return compile_access(sema, expr);

    default:
        ctu_assert(sema->reports, "(ctu) compile-expr unimplemented expr type %d", expr->type);
        return lir_poison(expr->node, "unimplemented");
    }
}

static lir_t *compile_cast(sema_t *sema, ctu_t *expr) {
    lir_t *it = compile_expr(sema, expr->src);
    type_t *type = compile_type(sema, expr->dst);

    it->type = type;

    return it;
}

/* actually compiles an rvalue */
lir_t *compile_expr(sema_t *sema, ctu_t *expr) {
    switch (expr->type) {
    case CTU_DIGIT: return compile_digit(expr);
    case CTU_BOOL: return compile_bool(expr);
    case CTU_UNARY: return compile_unary(sema, expr);
    case CTU_BINARY: return compile_binary(sema, expr);
    case CTU_CALL: return compile_call(sema, expr);
    case CTU_IDENT: return name_expr(expr->node, compile_name(sema, expr));
    case CTU_ACCESS: return name_expr(expr->node, compile_access(sema, expr));
    case CTU_STRING: return compile_string(expr);
    case CTU_CAST: return compile_cast(sema, expr);

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
    if (!is_discard(stmt->name)) {
        add_var(sema, stmt->name, value);
    }

    if (value->init) {
        return lir_assign(stmt->node, value, value->init);
    } 

    /* basically a no-op */
    return lir_stmts(stmt->node, vector_new(0));
}

static lir_t *compile_return(sema_t *sema, ctu_t *stmt) {
    lir_t *operand = NULL;
    if (stmt->operand != NULL) {
        operand = compile_expr(sema, stmt->operand);
    }

    return lir_return(stmt->node, operand);
}

static lir_t *compile_while(sema_t *sema, ctu_t *stmt) {
    lir_t *cond = compile_expr(sema, stmt->cond);
    lir_t *body = compile_stmt(sema, stmt->then);

    return lir_while(stmt->node, cond, body);
}

static lir_t *compile_assign(sema_t *sema, ctu_t *stmt) {
    lir_t *dst = compile_lvalue(sema, stmt->dst);
    lir_t *src = compile_expr(sema, stmt->src);

    if (is_const(lir_type(dst))) {
        report(sema->reports, ERROR, stmt->node, "cannot assign to const values");
    }

    return lir_assign(stmt->node, dst, src);
}

static lir_t *compile_branch(sema_t *sema, ctu_t *stmt) {
    lir_t *cond = compile_expr(sema, stmt->cond);
    lir_t *then = compile_stmt(sema, stmt->then);
    lir_t *other = NULL;

    if (stmt->other != NULL) {
        other = compile_stmts(sema, stmt->other);
    }

    return lir_branch(stmt->node, cond, then, other);
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
