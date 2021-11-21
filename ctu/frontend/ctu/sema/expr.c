#include "expr.h"
#include "type.h"
#include "value.h"
#include "define.h"
#include "sema.h"
#include "attrib.h"

static size_t SMALL_SIZES[TAG_MAX] = { MAP_SMALL, MAP_SMALL, MAP_SMALL };

static lir_t *compile_lvalue(sema_t *sema, ctu_t *expr);

static lir_t *compile_digit(ctu_t *digit) {
    return lir_digit(digit->node, 
        /* type = */ type_digit_with_name("int", SIGNED, TY_INT), 
        /* digit = */ digit->digit
    );
}

static lir_t *compile_bool(ctu_t *value) {
    return lir_bool(value->node, type_bool(), value->boolean);
}

static lir_t *compile_unary(sema_t *sema, ctu_t *expr) {
    lir_t *operand = lir_poison(expr->node, "unknown unary op");
    const type_t *type = NULL;
    unary_t unary = expr->unary;

    switch (unary) {
    case UNARY_ABS: case UNARY_NEG:
    case UNARY_BITFLIP:
        operand = compile_expr(sema, expr->operand);
        type = lir_type(operand);
        if (!is_integer(type)) {
            report(sema->reports, ERROR, expr->node, "unary math requires an integer operand, `%s` provided", type_format(type));
        }
        break;
    case UNARY_NOT:
        operand = compile_expr(sema, expr->operand);
        type = lir_type(operand);
        if (!is_bool(type)) {
            report(sema->reports, ERROR, expr->node, "unary not requires a boolean operand, `%s` provided", type_format(type));
        }
        break;
    case UNARY_DEREF:
        operand = compile_expr(sema, expr->operand);
        type = lir_type(operand);
        if (!is_pointer(type)) {
            report(sema->reports, ERROR, expr->node, "unary dereference requires a pointer operand, `%s` provided", type_format(type));
        } else {
            type = type->ptr;
        }
        break;
    case UNARY_ADDR:
        operand = compile_lvalue(sema, expr->operand);
        type = lir_type(operand);
        type = type_ptr(type);
        break;
    default:
        ctu_assert(sema->reports, "compile-unary unknown op %d", unary);
        break;
    }

    return lir_unary(expr->node, type, unary, operand);
}

static const type_t *common_math_digit(sema_t *sema, const type_t *lhs, const type_t *rhs) {
    if (!is_digit(lhs) || !is_digit(rhs)) {
        return NULL;
    }

    digit_t ldigit = lhs->digit;
    digit_t rdigit = rhs->digit;

    sign_t sign = (ldigit.sign == SIGNED || rdigit.sign == SIGNED) ? SIGNED : UNSIGNED;
    int_t width = MAX(ldigit.kind, rdigit.kind);

    return get_cached_digit_type(sema, sign, width);
}

static const type_t *common_bool(sema_t *sema, const type_t *lhs, const type_t *rhs) {
    if (!is_bool(lhs) || !is_bool(rhs)) {
        return NULL;
    }

    return get_cached_bool_type(sema);
}

/**
 * handle `==` and `!=` expressions
 */
static const type_t *binary_equal(sema_t *sema, node_t *node, lir_t *lhs, lir_t *rhs) {
    const type_t *type = common_math_digit(sema, lir_type(lhs), lir_type(rhs));
    const type_t *common = type == NULL ? common_bool(sema, lir_type(lhs), lir_type(rhs)) : type;

    if (common == NULL) {
        report(sema->reports, ERROR, node, "cannot take equality of `%s` and `%s`",
            type_format(lir_type(lhs)),
            type_format(lir_type(rhs))
        );

        return type_poison_with_node("invalid equality operands", node);
    }

    return get_cached_bool_type(sema);
}

/**
 * handle `<`, `<=`, `>`, and `>=` expressions
 */
static const type_t *binary_compare(sema_t *sema, node_t *node, lir_t *lhs, lir_t *rhs) {
    const type_t *type = common_math_digit(sema, lir_type(lhs), lir_type(rhs));
    if (type == NULL) {
        report(sema->reports, ERROR, node, "cannot compare `%s` to `%s`", 
            type_format(lir_type(lhs)), 
            type_format(lir_type(rhs))
        );
    }

    return get_cached_bool_type(sema);
}

/**
 * handle `&&` and `||` expressions
 */
static const type_t *binary_logic(sema_t *sema, node_t *node, lir_t *lhs, lir_t *rhs) {
    const type_t *type = common_bool(sema, lir_type(lhs), lir_type(rhs));
    if (type == NULL) {
        report(sema->reports, ERROR, node, "cannot logically evaluate `%s` with `%s`", 
            type_format(lir_type(lhs)), 
            type_format(lir_type(rhs))
        );

        return type_poison_with_node("invalid logic operands", node);
    }

    return type;
}

/**
 * handle `+`, `-`, `*`, and `/` expressions
 */
static const type_t *binary_math(sema_t *sema, node_t *node, lir_t *lhs, lir_t *rhs) {
    const type_t *type = common_math_digit(sema, lir_type(lhs), lir_type(rhs));
    if (type == NULL) {
        report(sema->reports, ERROR, node, "cannot perform math operations on `%s` with `%s`",
            type_format(lir_type(lhs)),
            type_format(lir_type(rhs))
        );

        return type_poison_with_node("invalid math operands", node);
    }

    return type;
}

static const type_t *binary_bits(sema_t *sema, node_t *node, lir_t *lhs, lir_t *rhs) {
    const type_t *type = common_math_digit(sema, lir_type(lhs), lir_type(rhs));
    if (type == NULL) {
        report(sema->reports, ERROR, node, "cannot perform bitwise operations on `%s` with `%s`",
            type_format(lir_type(lhs)),
            type_format(lir_type(rhs))
        );
        
        return type_poison_with_node("invalid bitwise operands", node);
    }

    return type;
}

static const type_t *binary_type(sema_t *sema, node_t *node, binary_t binary, lir_t *lhs, lir_t *rhs) {
    switch (binary) {
    case BINARY_EQ: case BINARY_NEQ:
        return binary_equal(sema, node, lhs, rhs);
    
    case BINARY_GT: case BINARY_GTE:
    case BINARY_LT: case BINARY_LTE:
        return binary_compare(sema, node, lhs, rhs);

    case BINARY_AND: case BINARY_OR:
        return binary_logic(sema, node, lhs, rhs);

    case BINARY_ADD: case BINARY_SUB:
    case BINARY_MUL: case BINARY_DIV: case BINARY_REM:
        return binary_math(sema, node, lhs, rhs);

    case BINARY_SHL: case BINARY_SHR:
    case BINARY_BITAND: case BINARY_BITOR: case BINARY_XOR:
        return binary_bits(sema, node, lhs, rhs);

    default:
        ctu_assert(sema->reports, "binary-type unreachable %d", binary);
        return type_poison_with_node("unreachable", node);
    }
}

static lir_t *compile_binary(sema_t *sema, ctu_t *expr) {
    lir_t *lhs = compile_expr(sema, expr->lhs);
    lir_t *rhs = compile_expr(sema, expr->rhs);
    binary_t binary = expr->binary;

    const type_t *common = binary_type(sema, expr->node, binary, lhs, rhs);

    return lir_binary(expr->node, common, binary, lhs, rhs);
}

static lir_t *compile_lambda(sema_t *sema, ctu_t *expr) {
    local_t save = move_state(sema);
    sema_t *nest = new_sema(sema->reports, sema, SMALL_SIZES);

    const type_t *type = lambda_type(sema, expr);
    set_return(nest, closure_result(type));
    add_locals(nest, type, expr->params);

    lir_t *body = compile_stmts(nest, expr->body);

    lir_t *lambda = ctu_forward(expr->node, NULL, LIR_DEFINE, NULL);
    lir_define(sema->reports, lambda,
        /* type = */ type,
        /* body = */ body
    );

    add_lambda(sema, lambda);

    delete_sema(nest);
    set_state(sema, save);

    return lambda;
}

static lir_t *compile_call(sema_t *sema, ctu_t *expr) {
    size_t len = vector_len(expr->args);
    lir_t *func = compile_lvalue(sema, expr->func);
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

static lir_t *read_expr(node_t *node, lir_t *lir) {
    if (lir_is(lir, LIR_PARAM) || lir_is(lir, LIR_DEFINE)) {
        return lir;
    }

    if (lir_is(lir, LIR_FORWARD) && lir->expected == LIR_DEFINE) {
        return lir;
    }

    return lir_read(node, lir_type(lir), lir);
}

static lir_t *compile_read(sema_t *sema, const char *name, ctu_t *expr) {
    if (is_discard(name)) {
        report(sema->reports, ERROR, expr->node, "reading from a discarded identifier `%s`", name);
    }
    
    lir_t *var = get_var(sema, name);
    if (var != NULL) {
        if (lir_is(var, LIR_PARAM) || lir_is(var, LIR_LOCAL)) {
            return var;
        }

        return compile_value(var);
    }

    lir_t *func = get_func(sema, name);
    if (func != NULL) {
        return compile_define(func);
    }

    report(sema->reports, ERROR, expr->node, "failed to resolve `%s`", name);
    return lir_poison(expr->node, "unresolved name");
}

static lir_t *compile_path(sema_t *sema, vector_t *path, ctu_t *expr) {
    size_t len = vector_len(path);

    if (len == 1) {
        return compile_read(sema, vector_head(path), expr);
    }

    sema_t *nest = get_module(sema, vector_head(path));
    if (nest != NULL) {
        return compile_path(nest, vector_slice(path, 1, len), expr);
    }

    report(sema->reports, ERROR, expr->node, "failed to find segment `%s`", (char*)vector_head(path));
    return lir_poison(expr->node, "unresolved path");
}

static lir_t *compile_string(ctu_t *expr) {
    return lir_string(expr->node, type_string(), expr->str);
}

static lir_t *compile_index(sema_t *sema, ctu_t *index) {
    lir_t *expr = compile_lvalue(sema, index->array);
    lir_t *subscript = compile_expr(sema, index->index);

    lir_t *it = implicit_convert_expr(sema, subscript, get_cached_usize(sema));
    if (it == NULL) {
        report(sema->reports, ERROR, index->node, "index type must be convertible to `usize`");
        return lir_poison(index->node, "invalid index type");
    }

    const type_t *exprtype = lir_type(expr);
    if (!type_can_index(exprtype)) {
        report(sema->reports, ERROR, index->node, "cannot index into type `%s`", type_format(exprtype));
        return lir_poison(index->node, "invalid array type");
    }

    const type_t *subtype = index_type(exprtype);

    return lir_offset(index->node, subtype, expr, it);
}

static lir_t *compile_list(sema_t *sema, ctu_t *expr) {
    const type_t *initial = NULL;
    if (expr->of != NULL) {
        initial = compile_type(sema, expr->of);
    }

    vector_t *items = expr->list;
    size_t len = vector_len(items);
    
    vector_t *result = vector_of(len);
    for (size_t i = 0; i < len; i++) {
        ctu_t *item = vector_get(items, i);
        lir_t *elem = compile_expr(sema, item);
    
        if (initial == NULL) {
            initial = lir_type(elem);
        } else {
            lir_t *elemcvt = implicit_convert_expr(sema, elem, initial);
            if (elemcvt == NULL) {
                report(sema->reports, ERROR, elem->node, "cannot convert list initializer element from `%s` to `%s`",
                    type_format(lir_type(elem)),
                    type_format(initial)
                );
            } else {
                elem = elemcvt;
            }
        }
        
        vector_set(result, i, elem);
    }

    return lir_list(expr->node, type_array(initial, len), result);
}

static lir_t *compile_null(ctu_t *expr) {
    return lir_null(expr->node, type_ptr(type_void()));
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

    case CTU_LAMBDA: return compile_lambda(sema, expr);
    case CTU_PATH: return compile_path(sema, expr->path, expr);
    case CTU_INDEX: return compile_index(sema, expr);
    case CTU_LIST: return compile_list(sema, expr);
    case CTU_NULL: return compile_null(expr);

    default:
        ctu_assert(sema->reports, "(ctu) compile-expr unimplemented expr type %d", expr->type);
        return lir_poison(expr->node, "unimplemented");
    }
}

static lir_t *compile_cast(sema_t *sema, ctu_t *expr) {
    lir_t *it = compile_expr(sema, expr->src);
    type_t *type = compile_type(sema, expr->dst);
    lir_t *cast = explicit_convert_expr(sema, it, type);

    if (cast == NULL) {
        report(sema->reports, ERROR, expr->node, "cannot convert `%s` to `%s`", type_format(lir_type(it)), type_format(type));
        return lir_poison(expr->node, "invalid cast");
    }

    return cast;
}

static lir_t *compile_sizeof(sema_t *sema, ctu_t *expr) {
    const type_t *type = compile_type(sema, expr->of);
    return lir_detail_sizeof(expr->node, type);
}

static lir_t *compile_alignof(sema_t *sema, ctu_t *expr) {
    const type_t *type = compile_type(sema, expr->of);
    return lir_detail_alignof(expr->node, type);
}

/* actually compiles an rvalue */
lir_t *compile_expr(sema_t *sema, ctu_t *expr) {
    switch (expr->type) {
    case CTU_DIGIT: return compile_digit(expr);
    case CTU_BOOL: return compile_bool(expr);
    case CTU_UNARY: return compile_unary(sema, expr);
    case CTU_BINARY: return compile_binary(sema, expr);
    case CTU_CALL: return compile_call(sema, expr);
    case CTU_STRING: return compile_string(expr);
    case CTU_CAST: return compile_cast(sema, expr);
    case CTU_LAMBDA: return compile_lambda(sema, expr);
    case CTU_NULL: return compile_null(expr);
    case CTU_INDEX: return read_expr(expr->node, compile_index(sema, expr));
    case CTU_PATH: return read_expr(expr->node, compile_path(sema, expr->path, expr));
    case CTU_LIST: return compile_list(sema, expr);
    case CTU_SIZEOF: return compile_sizeof(sema, expr);
    case CTU_ALIGNOF: return compile_alignof(sema, expr);

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
    
    if (!is_discard(stmt->name)) {
        add_var(sema, stmt->name, value);
    }

    return value;
}

static lir_t *compile_return(sema_t *sema, ctu_t *stmt) {
    lir_t *operand = NULL;
    lir_t *result = NULL;
    if (stmt->operand != NULL) {
        operand = compile_expr(sema, stmt->operand);

        const type_t *restype = get_return(sema);
        result = implicit_convert_expr(sema, operand, restype);

        if (result == NULL) {
            report(sema->reports, ERROR, operand->node, 
                "cannot return `%s` from a function that returns `%s`", 
                type_format(lir_type(operand)), type_format(restype)
            );
        } else if (is_void(lir_type(result))) {
            vector_t *stmts = vector_new(1);
            vector_push(&stmts, result);
            vector_push(&stmts, lir_return(stmt->node, NULL));
            return lir_stmts(stmt->node, stmts);
        }

        return lir_return(stmt->node, result);
    } else {
        return lir_return(stmt->node, NULL);
    }
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

static lir_t *compile_break(ctu_t *stmt) {
    return lir_break(stmt->node, NULL);
}

lir_t *compile_stmt(sema_t *sema, ctu_t *stmt) {
    switch (stmt->type) {
    case CTU_STMTS: return compile_stmts(new_sema(sema->reports, sema, SMALL_SIZES), stmt);
    case CTU_RETURN: return compile_return(sema, stmt);
    case CTU_WHILE: return compile_while(sema, stmt);
    case CTU_ASSIGN: return compile_assign(sema, stmt);
    case CTU_BRANCH: return compile_branch(sema, stmt);
    case CTU_BREAK: return compile_break(stmt);

    case CTU_CALL: return compile_call(sema, stmt);
    case CTU_VALUE: return compile_local(sema, stmt);

    case CTU_PATH:
        report(sema->reports, WARNING, stmt->node, "expression has no effect");
        return lir_stmts(stmt->node, vector_new(0));

    default:
        ctu_assert(sema->reports, "compile-stmt unknown type %d", stmt->type);
        return lir_poison(stmt->node, "compile-stmt unknown type");
    }
}
