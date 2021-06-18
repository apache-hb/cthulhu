#include "sema.h"

#include "cthulhu/util/report.h"
#include "cthulhu/front/compile.h"
#include "bison.h"

#include <string.h>

static nodes_t *ctx = NULL;

typedef struct {
    const char *name;
    node_t *node;
} decl_t;

typedef struct sema_t {
    struct sema_t *parent;

    decl_t *decls;
    size_t len, size;
} sema_t;

sema_t *new_sema(sema_t *parent) {
    sema_t *sema = malloc(sizeof(sema_t));
    sema->parent = parent;

    sema->decls = malloc(sizeof(decl_t) * 4);
    sema->len = 0;
    sema->size = 4;

    return sema;
}

static type_t *new_type(int kind, const char *name) {
    type_t *type = malloc(sizeof(type_t));
    type->kind = kind;
    type->name = name;
    return type;
}

static builtin_t builtin(int it, size_t width) {
    builtin_t ret = { it, width };
    return ret;
}

static type_t *new_builtin(builtin_t it, const char *name) {
    type_t *type = new_type(BUILTIN, name);
    type->builtin = it;
    return type;
}

static type_t *new_callable(const char *name, type_t *result) {
    type_t *type = new_type(CALLABLE, name);
    type->result = result;
    return type;
}

static type_t *poison(const char *msg, node_t *node) {
    type_t *type = new_type(POISON, msg);
    type->node = node;
    return type;
}

static type_t *LONG_TYPE = NULL;
static type_t *INT_TYPE = NULL;
static type_t *BOOL_TYPE = NULL;
static type_t *VOID_TYPE = NULL;

static type_t *current_return_type = NULL;
static node_t *current_func = NULL;

static bool types_equal(type_t *lhs, type_t *rhs) {
    if (lhs->kind == BUILTIN && rhs->kind == BUILTIN) {
        return lhs->builtin.real == rhs->builtin.real 
            && lhs->builtin.width == rhs->builtin.width;
    }

    return false;
}

static bool is_builtin_type(type_t *type, builtin_type_t real) {
    return type->kind == BUILTIN
        && type->builtin.real == real;
}

static bool is_integer_type(type_t *type) {
    return is_builtin_type(type, INTEGER);
}

static bool is_bool_type(type_t *type) {
    return is_builtin_type(type, BOOLEAN);
}
static type_t *get_typename(node_t *node) {
    const char *name = node->text;

    if (strcmp(name, "long") == 0)
        return LONG_TYPE;
    else if (strcmp(name, "bool") == 0)
        return BOOL_TYPE;
    else if (strcmp(name, "void") == 0)
        return VOID_TYPE;
    else if (strcmp(name, "int") == 0)
        return INT_TYPE;
    
    char *msg = format("unkonwn type `%s`", name);
    add_error(msg, node);
    return poison(msg, node);
}

static const char *nameof_type(type_t *type);

static char *callable_name(type_t *type) {
    const char *result = nameof_type(type->result);
    if (type->name) {
        return format("(function `%s` returning %s)", type->name, result);
    } else {
        return format("(function returning %s)", result);
    }
}

static const char *nameof_type(type_t *type) {
    switch (type->kind) {
    case BUILTIN: return type->name;
    case POISON: return type->name;
    case CALLABLE: return callable_name(type);
    default:
        reportf("nameof_type(type->kind = %d)", type->kind);
        return "unknown";
    }
}

static bool convertible_to(node_t *node, type_t *to, type_t *from) {
    if (types_equal(to, from))
        return true;

    if (is_integer_type(to) && is_integer_type(from)) {
        bool result = to->builtin.width >= from->builtin.width;
        
        if (!result && node) {
            const char *lhs_name = nameof_type(to),
                       *rhs_name = nameof_type(from);
            msg_idx_t id = add_error(
                format("cannot perfom narrowing conversion from %s to %s", rhs_name, lhs_name),
                node
            );
            add_note(id, format("%s is %zu bytes wide, %s is %zu bytes wide",
                lhs_name, to->builtin.width,
                rhs_name, from->builtin.width
            ));
        }

        return result;
    } 
    
    if (types_equal(to, BOOL_TYPE)) {
        return is_integer_type(from) || is_bool_type(from);
    }

    return false;
}

static type_t *typeof_node(sema_t *sema, node_t *node);

static bool is_type_bool_convertible(type_t *type) {
    return convertible_to(NULL, BOOL_TYPE, type);
}

static type_t *sema_binary(node_t *node, int op, type_t *lhs, type_t *rhs) {
    char *msg;
    msg_idx_t id;

    const char *lhs_name, *rhs_name;

    if (!types_equal(lhs, rhs)) {
        lhs_name = nameof_type(lhs);
        rhs_name = nameof_type(rhs);
        id = add_error("both sides of a binary expression must have the same type", node);
        msg = format("`%s` and `%s` are incompatible binary operands", lhs_name, rhs_name);
        add_note(id, msg);
        return poison(format("either %s or %s", lhs_name, rhs_name), node);
    }

    switch (op) {
    case ADD: case SUB: case DIV: 
    case MUL: case REM:
        if (!is_integer_type(lhs) || !is_integer_type(rhs)) {
            id = add_error("binary math must be performed on integer types", node);
            msg = format("%s and %s are nonsensical when performing arithmetic", nameof_type(lhs), nameof_type(rhs));
            add_note(id, msg);
            return poison(msg, node);
        }
        return lhs;
    
    default:
        reportf("sema_binary(op = %d)", op);
        return poison("sema_binary", node);
    }
}

static bool type_is_callable(type_t *type) {
    return type->kind == CALLABLE;
}

static void add_decl(sema_t *ctx, const char *name, node_t *decl) {
    ENSURE_SIZE(ctx->decls, ctx->len, ctx->size, sizeof(decl_t), 4);
    decl_t it = { name, decl };
    ctx->decls[ctx->len++] = it;
}

static node_t *find_decl(sema_t *ctx, const char *name) {
    for (size_t i = 0; i < ctx->len; i++) {
        decl_t decl = ctx->decls[i];
        if (strcmp(decl.name, name) == 0)
            return decl.node;
    }

    if (ctx->parent)
        return find_decl(ctx->parent, name);

    return NULL;
}

static type_t *typeof_node(sema_t *sema, node_t *node) {
    type_t *lhs, *rhs, *cond, *type;
    msg_idx_t id;
    node_t *it;
    type_t *out;

    /* short circuit for performance */
    if (node->typeof)
        return node->typeof;

    const char *msg, *lhs_name, *rhs_name;
    switch (node->type) {
    case AST_DIGIT: out = INT_TYPE; break;
    case AST_BOOL: out = BOOL_TYPE; break;
    case AST_IDENT:
        it = find_decl(sema, node->text);
        if (!it) {
            add_error(format("could not resolve `%s`", node->text), node);
            return poison(format("unresolved symbol `%s`", node->text), node);
        }
        out = typeof_node(sema, it);
        break;
    case AST_BINARY: 
        lhs = typeof_node(sema, node->binary.lhs);
        rhs = typeof_node(sema, node->binary.rhs);
        out = sema_binary(node, node->binary.op, lhs, rhs);
        break;
    case AST_TERNARY:
        cond = typeof_node(sema, node->ternary.cond);
        lhs = typeof_node(sema, node->ternary.lhs);
        rhs = typeof_node(sema, node->ternary.rhs);

        if (!types_equal(lhs, rhs)) {
            lhs_name = nameof_type(lhs);
            rhs_name = nameof_type(rhs);
            id = add_error("both sides of ternary must be the same type", node);
            msg = format("%s and %s are incompatible ternary leafs", lhs_name, rhs_name);
            add_note(id, msg);
            out = poison(format("either `%s` or `%s`", lhs_name, rhs_name), node);
        }

        if (!is_type_bool_convertible(cond)) {
            add_error("ternary condition must be convertible to a boolean", node->ternary.cond);
        }

        out = lhs;
        break;
    case AST_TYPENAME:
        out = get_typename(node);
        break;
    case AST_FUNC:
        out = new_callable(node->func.name, typeof_node(sema, node->func.result));
        break;
    case AST_CALL:
        type = typeof_node(sema, node->expr);
        if (!type_is_callable(type)) {
            add_error("cannot call non-invokable expression", node);
            return poison("a malformed function call", node);
        }
        out = type->result;
        break;
    case AST_VAR:
        out = typeof_node(sema, node->var.init);
        break;
    case AST_CAST:
        out = typeof_node(sema, node->cast.type);
        type = typeof_node(sema, node->cast.expr);
        if (!convertible_to(node, out, type)) {
            add_error(
                format("cannot cast %s to %s", nameof_type(type), nameof_type(out)), 
                node
            );
        }
        break;
    default:
        reportf("typeof_node(node->type = %d)", node->type);
        out = VOID_TYPE;
        break;
    }

    node->typeof = out;
    return out;
}

static void sema_node(sema_t *sema, node_t *node) {
    size_t i = 0;
    msg_idx_t msg;
    type_t *type;
    sema_t *nest;

    const char *ret_name, *current_name;

    switch (node->type) {
    case AST_DIGIT: 
    case AST_IDENT: 
    case AST_BOOL:
        break;

    case AST_CALL:
        type = typeof_node(sema, node->expr);
        if (!type_is_callable(type)) {
            add_error(format("%s is not callable", nameof_type(type)), node->expr);
        }
        break;

    case AST_UNARY:
        sema_node(sema, node->unary.expr);
        break;

    case AST_BINARY:
        typeof_node(sema, node);
        msg = add_warn("discarding result of binary expression", node);
        add_note(msg, "assign this expression to a value to suppress this warning");
        break;

    case AST_RETURN:
        if (!node->expr) {
            if (!types_equal(current_return_type, VOID_TYPE)) {
                add_error("non-void function cannot return void", node);
            }
        } else {
            type = typeof_node(sema, node->expr);
            if (!convertible_to(node, current_return_type, type)) {
                current_name = nameof_type(current_return_type);
                ret_name = nameof_type(type);
                msg = add_error("type mismatch in return", node);
                add_note(msg, format("function `%s` returns `%s` but %s was provided", 
                    current_func->func.name,
                    current_name, ret_name
                ));
            } else {
                node->expr = ast_cast(node->source, node->loc, node, current_func->func.result);
            }
        }
        break;

    /** 
     * if for some reason a user decides to use a ternary as a statement
     * we handle that by typechecking it
     * (why someone would discard the result of a ternary is beyond me, but they can)
     */
    case AST_TERNARY:
        typeof_node(sema, node);
        msg = add_warn("discarding result of ternary expression", node);
        add_note(msg, "assign this expression to a value to suppress this warning");
        break;

    case AST_STMTS: 
        nest = new_sema(sema);
        for (i = 0; i < node->stmts->len; i++)
            sema_node(nest, node->stmts->data + i);
        break;

    case AST_FUNC:
        reportf("sema_node(AST_FUNC)");
        break;

    case AST_TYPENAME:
        reportf("sema_node(AST_TYPENAME)");
        break;

    case AST_VAR:
        add_decl(sema, node->var.name, node->var.init);
        typeof_node(sema, node->var.init);
        break;

    case AST_CAST:
        typeof_node(sema, node);
        break;
    }
}

static void sema_func(sema_t *sema, node_t *node) {
    /* functions shouldnt be nested with this setup */
    ASSERT(current_func == NULL);
    ASSERT(current_return_type == NULL);

    current_func = node;
    current_return_type = typeof_node(sema, node->func.result);

    sema_node(sema, node->func.body);

    current_func = NULL;
    current_return_type = NULL;
}

void sema_mod(nodes_t *nodes) {
    ctx = nodes;

    LONG_TYPE = new_builtin(builtin(INTEGER, 8), "long");
    INT_TYPE = new_builtin(builtin(INTEGER, 4), "int");
    BOOL_TYPE = new_builtin(builtin(BOOLEAN, 0), "bool");
    VOID_TYPE = new_builtin(builtin(VOID, 0), "void");
    
    sema_t *sema = new_sema(NULL);

    for (size_t i = 0; i < nodes->len; i++) {
        node_t *node = nodes->data + i;

        if (node->type == AST_FUNC) 
            add_decl(sema, node->func.name, node);
    }

    for (size_t i = 0; i < nodes->len; i++) {
        sema_func(sema, nodes->data + i);
    }
}
