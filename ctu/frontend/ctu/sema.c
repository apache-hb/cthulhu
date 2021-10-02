#include "sema.h"
#include "ctu/util/util.h"
#include "ctu/util/report-ext.h"

#if 0
typedef enum {
    TAG_TYPES,
    TAG_VARS,
    TAG_FUNCS,

    TAG_MAX
} ctu_tag_t;

typedef struct {
    bool building;
    ctu_t *decl;
    sema_t *sema;
} ctx_t;

static ctx_t *ctx_new(ctu_t *decl, sema_t *sema) {
    ctx_t *ctx = ctu_malloc(sizeof(ctx_t));
    ctx->building = false;
    ctx->decl = decl;
    ctx->sema = sema;
    return ctx;
}

#define NEW_SEMA(parent, reports, sizes) \
    sema_new(parent, reports, TAG_MAX, sizes)

#define DELETE_SEMA(sema) \
    sema_delete(sema)

static leaf_t ctu_leaf(ctu_t *decl) {
    switch (decl->type) {
    case CTU_VALUE: return LIR_VALUE;
    default: return LIR_POISON;
    }
}

static ctu_tag_t ctu_tag(ctu_t *decl) {
    switch (decl->type) {
    case CTU_VALUE: return TAG_VARS;
    default: return TAG_VARS;
    }
}

static lir_t *ctu_declare(sema_t *sema, ctu_t *decl) {
    leaf_t leaf = ctu_leaf(decl);
    return lir_forward(decl->node, decl->name, leaf, ctx_new(decl, sema));
}

static void add_global(sema_t *sema, ctu_tag_t tag, const char *name, lir_t *lir) {
    lir_t *var = sema_get(sema, TAG_VARS, name);
    if (var != NULL) {
        report_shadow(sema->reports, name, var->node, lir->node);
    }

    lir_t *func = sema_get(sema, TAG_FUNCS, name);
    if (func != NULL) {
        report_shadow(sema->reports, name, func->node, lir->node);
    }

    sema_set(sema, tag, name, lir);
}

static lir_t *compile_expr(sema_t *sema, ctu_t *expr);

static type_t *get_type(sema_t *sema, const char *name) {
    return sema_get(sema, TAG_TYPES, name);
}

static void add_type(sema_t *sema, const char *name, type_t *type) {
    type_t *other = get_type(sema, name);
    if (other != NULL) {
        report(sema->reports, ERROR, NULL, "declaration of %s shadows existing type", name);
    }

    sema_set(sema, TAG_TYPES, name, type);
}

static type_t *compile_type(sema_t *sema, ctu_t *node);

static type_t *compile_pointer_type(sema_t *sema, ctu_t *node) {
    type_t *to = compile_type(sema, node->ptr);

    return type_ptr(to);
}

static type_t *compile_name_type(sema_t *sema, ctu_t *node) {
    const char *id = node->ident;

    type_t *real = get_type(sema, id);
    if (real == NULL) {
        report(sema->reports, ERROR, node->node, "unknown type `%s`", id);
        real = type_poison("unresolved type");
    }

    return real;
}

static type_t *compile_type(sema_t *sema, ctu_t *node) {
    switch (node->type) {
    case CTU_POINTER: return compile_pointer_type(sema, node);
    case CTU_TYPENAME: return compile_name_type(sema, node);

    default:
        ctu_assert(sema->reports, "compile-type unknown node %d", node->type);
        return type_poison("unknown type");
    }
}

static lir_t *compile_expr(sema_t *sema, ctu_t *expr);
static lir_t *compile_value(sema_t *sema, lir_t *decl);

static lir_t *compile_digit(ctu_t *expr) {
    return lir_digit(expr->node, type_literal_integer(), expr->digit);
}

static lir_t *compile_name(sema_t *sema, ctu_t *expr) {
    const char *name = expr->ident;
    lir_t *value = sema_get(sema, TAG_VARS, name);
    if (value != NULL) {
        return lir_name(expr->node, compile_value(sema, value));
    }

    lir_t *func = sema_get(sema, TAG_FUNCS, name);
    if (func != NULL) {
        return lir_name(expr->node, func);
    }

    report(sema->reports, ERROR, expr->node, "failed to resolve name `%s`", name);
    return lir_poison(expr->node, "unresolved name");
}

static lir_t *compile_unary(sema_t *sema, ctu_t *expr) {
    lir_t *operand = compile_expr(sema, expr->operand);
    unary_t op = expr->unary;

    return lir_unary(expr->node, lir_type(operand), op, operand);
}

static lir_t *compile_binary(sema_t *sema, ctu_t *expr) {
    lir_t *lhs = compile_expr(sema, expr->lhs),
          *rhs = compile_expr(sema, expr->rhs);
        
    binary_t op = expr->binary;

    type_t *common = types_common(lir_type(lhs), lir_type(rhs));

    return lir_binary(expr->node, common, op, lhs, rhs);
}

static lir_t *compile_expr(sema_t *sema, ctu_t *expr) {
    switch (expr->type) {
    case CTU_DIGIT: 
        return compile_digit(expr);
    case CTU_IDENT:
        return compile_name(sema, expr);
    case CTU_UNARY:
        return compile_unary(sema, expr);
    case CTU_BINARY:
        return compile_binary(sema, expr);
    
    default:
        ctu_assert(sema->reports, "unexpected expr type %d", expr->type);
        return lir_poison(expr->node, "unexpected expr type");
    }
}

static void compile_typedecl(sema_t *sema, lir_t *decl) {
    UNUSED(sema);
    UNUSED(decl);
}

static lir_t *not_zero(lir_t *expr) {
    lir_t *zero = lir_int(expr->node, lir_type(expr), 0);
    lir_t *cmp = lir_binary(expr->node, type_bool(), BINARY_NEQ, expr, zero);

    return cmp;
}

static lir_t *implicit_bool(lir_t *expr) {
    const type_t *type = lir_type(expr);
    if (is_bool(type)) {
        return expr;
    }

    if (is_digit(type)) {
        return not_zero(expr);
    }

    return NULL;
}

static lir_t *implicit_digit(const type_t *to, lir_t *expr) {
    const type_t *from = lir_type(expr);

    if (is_literal(from)) {
        expr->type = to;
        return expr;
    }

    return NULL;
}

static lir_t *implicit_cast(lir_t *expr, const type_t *type) {
    if (is_bool(type)) {
        return implicit_bool(expr);
    }

    if (is_digit(type)) {
        return implicit_digit(type, expr);
    }

    return NULL;
}

static lir_t *realise_expr(lir_t *expr) {
    const type_t *type = lir_type(expr);
    if (is_literal(type)) {
        expr->type = type_digit(SIGNED, TY_INT);
    }
    return expr;
}

static lir_t *compile_value(sema_t *sema, lir_t *decl) {
    if (!lir_is(decl, LIR_FORWARD)) {
        return decl;
    }

    ctx_t *ctx = decl->ctx;
    ctu_t *node = ctx->decl;
    node_t *where = node->node;

    if (ctx->building) {
        return lir_poison(where, "recursive dependency");
    }

    ctx->building = true;

    lir_t *init = node->value ? compile_expr(sema, node->value) : NULL;
    const type_t *type = node->kind ? compile_type(sema, node->kind) : NULL;

    if (init != NULL) {
        vector_t *path = lir_recurses(init, decl);
        if (path != NULL) {
            report_recursive(sema->reports, path, decl);
        } else {
            init = realise_expr(init);
        }
    }

    if (type != NULL && is_void(type)) {
        report(sema->reports, ERROR, where,
            "value `%s` cannot be a void type", 
            node->name
        );
    }

    if (init != NULL) {
        if (type != NULL) {
            lir_t *cast = implicit_cast(init, type);
            if (cast == NULL) {
                report(sema->reports, ERROR, where, "cannot implitly convert from `%s` to `%s`",
                    /* provided-type = */ type_format(lir_type(init)),
                    /* expected-type = */ type_format(type)
                );
            } else {
                init = cast;
            }
        }

        type = lir_type(init);
    }

    lir_value(sema->reports,
        /* dst = */ decl,
        /* type = */ type,
        /* init = */ init
    );

    ctx->building = false;

    return decl;
}

static void build_value(sema_t *sema, lir_t *decl) {
    compile_value(sema, decl);
}

static void compile_func(sema_t *sema, lir_t *decl) {
    size_t sizes[TAG_MAX] = {
        [TAG_TYPES] = MAP_SMALL,
        [TAG_VARS] = MAP_SMALL,
        [TAG_FUNCS] = MAP_SMALL
    };
    sema_t *nest = NEW_SEMA(sema, sema->reports, sizes);

    ctx_t *ctx = decl->ctx;
    ctu_t *it = ctx->decl;

    UNUSED(it);

    DELETE_SEMA(nest);
}

static sema_t *base_sema(reports_t *reports, size_t decls) {
    size_t sizes[TAG_MAX] = {
        [TAG_TYPES] = decls,
        [TAG_VARS] = decls,
        [TAG_FUNCS] = decls
    };

    sema_t *sema = NEW_SEMA(NULL, reports, sizes);

    add_type(sema, "int", type_digit(SIGNED, TY_INT));
    add_type(sema, "uint", type_digit(UNSIGNED, TY_INT));
    add_type(sema, "void", type_void());
    add_type(sema, "bool", type_bool());

    return sema;
}

lir_t *ctu_sema(reports_t *reports, ctu_t *ctu) {
    vector_t *decls = ctu->decls;
    size_t ndecls = vector_len(decls);

    sema_t *sema = base_sema(reports, ndecls);

    for (size_t i = 0; i < ndecls; i++) {
        ctu_t *decl = vector_get(decls, i);
        lir_t *lir = ctu_declare(sema, decl);
        add_global(sema, ctu_tag(decl), decl->name, lir);
    }

    map_t *type_map = sema_tag(sema, TAG_TYPES);
    map_t *global_map = sema_tag(sema, TAG_VARS);
    map_t *func_map = sema_tag(sema, TAG_FUNCS);

    MAP_APPLY(type_map, sema, compile_typedecl);
    MAP_APPLY(global_map, sema, build_value);
    MAP_APPLY(func_map, sema, compile_func);
    
    vector_t *funcs = map_values(func_map);
    vector_t *vars = map_values(global_map);

    DELETE_SEMA(sema);

    return lir_module(ctu->node,
        /* imports = */ vector_of(0),
        /* types = tys, */
        vars,
        funcs
    );
}
#endif

typedef enum {
    TAG_TYPES,
    TAG_GLOBALS,
    TAG_FUNCS,

    TAG_MAX
} tag_t;

typedef struct {
    void *TODO;
} stack_t;

static stack_t *stack_new(void) {
    stack_t *stack = ctu_malloc(sizeof(stack_t));
    
    return stack;
}

static sema_t *new_sema(reports_t *reports, 
                        sema_t *parent,
                        size_t *sizes) {
    sema_t *sema = sema_new(parent, reports, TAG_MAX, sizes);
    sema_set_data(sema, stack_new());
    return sema;
}

static lir_t *compile_value(sema_t *sema, lir_t *lir);
static lir_t *compile_expr(sema_t *sema, lir_t *lir);
static void build_expr(sema_t *sema, lir_t *lir);

static lir_t *compile_value(sema_t *sema, lir_t *lir) {

}

static lir_t *compile_expr(sema_t *sema, lir_t *lir) {

}

static void build_expr(sema_t *sema, lir_t *lir) {

}

static sema_t *base_sema(reports_t *reports, size_t decls) {
    size_t sizes[TAG_MAX] = {
        [TAG_TYPES] = decls,
        [TAG_GLOBALS] = decls,
        [TAG_FUNCS] = decls
    };

    sema_t *sema = new_sema(reports, NULL, sizes);

    /*
    add_type(sema, "int", type_digit(SIGNED, TY_INT));
    add_type(sema, "uint", type_digit(UNSIGNED, TY_INT));
    add_type(sema, "void", type_void());
    add_type(sema, "bool", type_bool());
    */

    return sema;
}

lir_t *ctu_sema(reports_t *reports, ctu_t *ctu) {
    vector_t *decls = ctu->decls;
    size_t ndecls = vector_len(decls);
    sema_t *sema = base_sema(reports, ndecls);

    sema_delete(sema);
    
    ctu_assert(reports, "ctu-sema unimplemented");
    return NULL;
}
