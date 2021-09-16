#include "sema.h"

#include "ctu/util/report.h"

#include <ctype.h>

typedef enum {
    TAG_VARS,
    TAG_CONSTS,
    TAG_PROCS,

    TAG_MAX
} pl0_tag_t;

#define NEW_SEMA(parent, reports) \
    sema_new(parent, reports, TAG_MAX)
    
#define DELETE_SEMA(sema) \
    sema_delete(sema)

static void report_shadow(reports_t *reports, const char *name, node_t *other, node_t *self) {
    message_t *id = report2(reports, ERROR, self, "refinition of `%s`", name);
    report_append2(id, other, "previous definition");
    report_note2(id, "PL/0 is case insensitive");
}

static void report_recurse(reports_t *reports, vector_t *stack, lir_t *root) {
    node_t *node = root->node;
    message_t *id = report2(reports, ERROR, node, "initialization of `%s` is recursive", root->name);
    
    node_t *last = node;
    size_t len = vector_len(stack);
    size_t t = 0;

    for (size_t i = 0; i < len; i++) {
        node_t *it = vector_get(stack, i);
        if (it != last) {
            report_append2(id, it, "trace %zu", t++);
        }
        last = it;
    }
}

static lir_t *compile_expr(sema_t *sema, pl0_t *expr);
static lir_t *compile_stmt(sema_t *sema, pl0_t *stmt);

static char *pl0_name(const char *name) {
    char *out = ctu_strdup(name);
    for (char *p = out; *p != '\0'; p++) {
        *p = tolower(*p);
    }
    return out;
}

static type_t *pl0_int(bool mut) {
    type_t *ty = type_digit(SIGNED, TY_LONG);
    type_mut(ty, mut);
    return ty;
}

static type_t *pl0_bool(void) {
    return type_bool();
}

static type_t *pl0_closure(void) {
    vector_t *args = vector_new(0);
    return type_closure(args, type_void());
}

static lir_t *pl0_num(node_t *node, int num) {
    return lir_int(node, pl0_int(false), num);
}

static lir_t *compile_digit(pl0_t *expr) {
    return lir_digit(expr->node, pl0_int(false), expr->digit);
}

static lir_t *query_ident(sema_t *sema, const char *name) {
    lir_t *val = sema_get(sema, TAG_CONSTS, name);
    if (val != NULL) {
        return val;
    }

    lir_t *var = sema_get(sema, TAG_VARS, name);
    if (var != NULL) {
        return var;
    }

    return NULL;
}

static void set_proc(sema_t *sema, const char *name, lir_t *proc) {
    lir_t *other = sema_get(sema, TAG_PROCS, name);
    if (other != NULL) {
        report_shadow(sema->reports, name, other->node, proc->node);
    }

    sema_set(sema, TAG_PROCS, name, proc);
}

static void set_var(sema_t *sema, size_t tag, const char *name, lir_t *var) {
    lir_t *other1 = sema_get(sema, TAG_CONSTS, name);
    if (other1 != NULL) {
        report_shadow(sema->reports, name, other1->node, var->node);
    }

    lir_t *other2 = sema_get(sema, tag, name);
    if (other2 != NULL) {
        report_shadow(sema->reports, name, other2->node, var->node);
    }

    sema_set(sema, tag, name, var);
}

static lir_t *compile_ident(sema_t *sema, pl0_t *expr) {
    char *name = pl0_name(expr->ident);
    node_t *node = expr->node;

    lir_t *val = query_ident(sema, name);

    if (val != NULL) {
        return lir_name(node, val);
    }

    message_t *id = report2(sema->reports, ERROR, node, "unknown variable name `%s`", name);
    report_note2(id, "PL/0 is case insensitive");

    return lir_poison(node, "unresolved variable");
}

static lir_t *compile_binary(sema_t *sema, pl0_t *expr) {
    lir_t *lhs = compile_expr(sema, expr->lhs);
    lir_t *rhs = compile_expr(sema, expr->rhs);

    type_t *type = types_common(lir_type(lhs), lir_type(rhs));

    return lir_binary(expr->node, type, expr->binary, lhs, rhs);
}

static lir_t *compile_unary(sema_t *sema, pl0_t *expr) {
    lir_t *operand = compile_expr(sema, expr->operand);

    return lir_unary(expr->node, lir_type(operand), expr->unary, operand);
}

static lir_t *compile_odd(sema_t *sema, pl0_t *expr) {
    lir_t *operand = compile_expr(sema, expr->operand);

    node_t *node = expr->node;
    lir_t *rem = lir_binary(node, operand->type, BINARY_REM, operand, pl0_num(node, 2));
    lir_t *cmp = lir_binary(node, pl0_bool(), BINARY_EQ, rem, pl0_num(node, 1));
    return cmp;
}

static lir_t *compile_expr(sema_t *sema, pl0_t *expr) {
    switch (expr->type) {
    case PL0_DIGIT: 
        return compile_digit(expr);
    case PL0_IDENT:
        return compile_ident(sema, expr);
    case PL0_BINARY:
        return compile_binary(sema, expr);
    case PL0_UNARY:
        return compile_unary(sema, expr);
    default:
        assert2(sema->reports, "(pl0) compile-expr unknown expr %d", expr->type);
        return lir_poison(expr->node, "unknown expr");
    }
}

static lir_t *compile_cmp(sema_t *sema, pl0_t *expr) {
    lir_t *lhs = compile_expr(sema, expr->lhs);
    lir_t *rhs = compile_expr(sema, expr->rhs);

    return lir_binary(expr->node, pl0_bool(), expr->binary, lhs, rhs);
}

static lir_t *compile_cond(sema_t *sema, pl0_t *expr) {
    switch (expr->type) {
    case PL0_ODD:
        return compile_odd(sema, expr);
    case PL0_BINARY:
        return compile_cmp(sema, expr);
    
    default:
        assert2(sema->reports, "compile-cond unknown cond %d", expr->type);
        return lir_poison(expr->node, "unknown cond");
    }
}

static lir_t *compile_assign(sema_t *sema, pl0_t *stmt) {
    node_t *node = stmt->node;
    char *name = pl0_name(stmt->dst);
    lir_t *lhs = query_ident(sema, name);
    if (lhs == NULL) {
        message_t *id = report2(sema->reports, ERROR, node, "unknown variable name `%s`", name);
        report_note2(id, "PL/0 is case insensitive");

        return lir_poison(node, "unresolved variable");
    }

    lir_t *rhs = compile_expr(sema, stmt->src);

    if (!lhs->type->mut) {
        report2(sema->reports, ERROR, node, "cannot assign to const value `%s` %s", name, type_format(lhs->type));
    }

    return lir_assign(node, lhs, rhs);
}

static lir_t *compile_print(sema_t *sema, pl0_t *stmt) {
    lir_t *expr = compile_expr(sema, stmt->operand);

    return expr;
}

static lir_t *compile_loop(sema_t *sema, pl0_t *stmt) {
    lir_t *cond = compile_cond(sema, stmt->cond);
    lir_t *then = compile_stmt(sema, stmt->then);

    return lir_while(stmt->node, cond, then);
}

static lir_t *compile_stmts(sema_t *sema, pl0_t *stmts) {
    vector_t *all = stmts->stmts;
    size_t len = vector_len(all);
    vector_t *result = vector_of(len);

    for (size_t i = 0; i < len; i++) {
        pl0_t *stmt = vector_get(all, i);
        lir_t *lir = compile_stmt(sema, stmt);
        vector_set(result, i, lir);
    }

    return lir_stmts(stmts->node, result);
}

static lir_t *compile_branch(sema_t *sema, pl0_t *stmt) {
    lir_t *cond = compile_cond(sema, stmt->cond);
    lir_t *then = compile_stmt(sema, stmt->then);

    return lir_branch(stmt->node, cond, then, NULL);
}

static lir_t *compile_call(sema_t *sema, pl0_t *stmt) {
    char *name = pl0_name(stmt->ident);
    lir_t *proc = sema_get(sema, TAG_PROCS, name);
    
    if (proc == NULL) {
        report2(sema->reports, ERROR, stmt->node, "unknown procedure `%s`", name);
        return lir_poison(stmt->node, "unknown procedure");
    }

    return lir_call(stmt->node, proc, vector_new(0));
}

static lir_t *compile_stmt(sema_t *sema, pl0_t *stmt) {
    switch (stmt->type) {
    case PL0_ASSIGN:
        return compile_assign(sema, stmt);
    case PL0_PRINT:
        return compile_print(sema, stmt);
    case PL0_LOOP:
        return compile_loop(sema, stmt);
    case PL0_STMTS:
        return compile_stmts(sema, stmt);
    case PL0_BRANCH:
        return compile_branch(sema, stmt);
    case PL0_CALL:
        return compile_call(sema, stmt);

    default:
        assert2(sema->reports, "compile-stmt unknown stmt %d", stmt->type);
        return lir_poison(stmt->node, "unknown stmt");
    }
}

static void compile_const(sema_t *sema, lir_t *lir) {
    pl0_t *node = lir->ctx;
    lir_t *value = compile_expr(sema, node->value);

    vector_t *path = lir_recurses(value, lir);
    if (path != NULL) {
        report_recurse(sema->reports, path, lir);
    }

    lir_value(sema->reports, lir, pl0_int(false), value);
}

static lir_t *build_var(sema_t *sema, pl0_t *node) {
    char *name = pl0_name(node->name);
    lir_t *lir = lir_forward(node->node, name, LIR_VALUE, NULL);
    lir_value(sema->reports, lir, pl0_int(true), pl0_num(node->node, 0));
    
    sema_set(sema, TAG_VARS, name, lir);

    return lir;
}

static void compile_var(sema_t *sema, lir_t *lir) {
    UNUSED(sema);

    pl0_t *node = lir->ctx;
    lir_t *value = pl0_num(node->node, 0);

    lir_value(sema->reports, lir, pl0_int(true), value);
}

static void compile_proc(sema_t *sema, lir_t *lir) {
    UNUSED(sema);

    pl0_t *node = lir->ctx;
    size_t nlocals = vector_len(node->locals);
    vector_t *locals = vector_of(nlocals);

    sema_t *nest = NEW_SEMA(sema, sema->reports);

    for (size_t i = 0; i < nlocals; i++) {
        pl0_t *local = vector_get(node->locals, i);
        lir_t *var = build_var(nest, local);
        vector_set(locals, i, var);
    }

    size_t nbody = vector_len(node->body);
    vector_t *body = vector_of(nbody);

    for (size_t i = 0; i < nbody; i++) {
        pl0_t *it = vector_get(node->body, i);
        lir_t *stmt = compile_stmt(nest, it);
        vector_set(body, i, stmt);
    }

    lir_define(sema->reports, lir, pl0_closure(), locals, lir_stmts(node->node, body));
}

static lir_t *pl0_declare(pl0_t *pl0, const char *name, leaf_t leaf) {
    return lir_forward(pl0->node, name, leaf, pl0);
}

static bool always(void *value) {
    return value != NULL;
}

lir_t *pl0_sema(reports_t *reports, pl0_t *node) {
    sema_t *sema = NEW_SEMA(NULL, reports);

    vector_t *vars = node->globals;
    vector_t *consts = node->consts;
    vector_t *procs = node->procs;

    size_t nvars = vector_len(vars);
    size_t nconsts = vector_len(consts);
    size_t nprocs = vector_len(procs);

    for (size_t i = 0; i < nvars; i++) {
        pl0_t *decl = vector_get(vars, i);
        char *name = pl0_name(decl->name);
        set_var(sema, TAG_VARS, name, pl0_declare(decl, name, LIR_VALUE));
    }

    for (size_t i = 0; i < nconsts; i++) {
        pl0_t *decl = vector_get(consts, i);
        char *name = pl0_name(decl->name);
        set_var(sema, TAG_CONSTS, name, pl0_declare(decl, name, LIR_VALUE));
    }

    for (size_t i = 0; i < nprocs; i++) {
        pl0_t *decl = vector_get(procs, i);
        char *name = pl0_name(decl->name);
        set_proc(sema, name, pl0_declare(decl, name, LIR_DEFINE));
    }

    pl0_t *top = node->toplevel;
    if (top != NULL) {
        /* TODO: fix this */
        node_t *node = top->node;
        vector_t *body = vector_init(top);
        pl0_t *entry = pl0_procedure(node->scan, node->where, "pl0-main", vector_new(0), body);
        set_proc(sema, "pl0-main", pl0_declare(entry, "pl0-main", LIR_DEFINE));
    }

    map_t *const_map = sema_tag(sema, TAG_CONSTS);
    map_t *var_map = sema_tag(sema, TAG_VARS);
    map_t *proc_map = sema_tag(sema, TAG_PROCS);

    MAP_APPLY(const_map, sema, compile_const);
    MAP_APPLY(var_map, sema, compile_var);
    MAP_APPLY(proc_map, sema, compile_proc);

    vector_t *globals = vector_join(
        MAP_COLLECT(var_map, always),
        MAP_COLLECT(const_map, always)
    );

    vector_t *funcs = MAP_COLLECT(proc_map, always);

    DELETE_SEMA(sema);

    return lir_module(node->node, globals, funcs);
}
