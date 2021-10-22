#include "sema.h"

#include "ctu/util/report.h"
#include "ctu/util/report-ext.h"
#include "ctu/util/str.h"

#include <string.h>
#include <ctype.h>

typedef enum {
    TAG_VARS,
    TAG_CONSTS,
    TAG_PROCS,

    TAG_MAX
} pl0_tag_t;

static char *pl0_name(const char *name) {
    char *out = ctu_strdup(name);
    for (char *p = out; *p != '\0'; p++) {
        *p = tolower(*p);
    }
    return out;
}

#define NEW_SEMA(parent, path, reports, sizes) \
    sema_new(parent, path, reports, TAG_MAX, sizes)
    
#define DELETE_SEMA(sema) \
    sema_delete(sema)

static const attrib_t PRINT = {
    .visibility = IMPORTED,
    .mangle = "printf"
};

static lir_t *pl0_import_print(reports_t *reports, node_t *node) {
    vector_t *args = vector_init(type_string());
    vector_push(&args, type_varargs());
    
    const type_t *type = type_closure(args, type_digit(SIGNED, TY_INT));
    
    lir_t *func = lir_forward(node, "printf", LIR_DEFINE, NULL);
    lir_define(reports, func, type, vector_new(0), NULL);
    lir_attribs(func, &PRINT);

    return func;
}

static void pl0_shadow(reports_t *reports, 
                       const char *name, 
                       node_t *other, 
                       node_t *self)
{
    message_t *id = report_shadow(reports, name, other, self);
    report_note(id, "PL/0 is case insensitive");
}

static lir_t *compile_expr(sema_t *sema, pl0_t *expr);
static lir_t *compile_stmt(sema_t *sema, pl0_t *stmt);

static type_t *pl0_int(bool mut) {
    return type_mut(type_digit(SIGNED, TY_LONG), mut);
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
        pl0_shadow(sema->reports, name, other->node, proc->node);
    }

    sema_set(sema, TAG_PROCS, name, proc);
}

static void set_var(sema_t *sema, size_t tag, const char *name, lir_t *var) {
    lir_t *other1 = sema_get(sema, TAG_CONSTS, name);
    if (other1 != NULL) {
        pl0_shadow(sema->reports, name, var->node, other1->node);
    }

    lir_t *other2 = sema_get(sema, TAG_VARS, name);
    if (other2 != NULL) {
        pl0_shadow(sema->reports, name, var->node, other2->node);
    }

    sema_set(sema, tag, name, var);
}

static lir_t *compile_ident(sema_t *sema, pl0_t *expr) {
    char *name = pl0_name(expr->ident);
    node_t *node = expr->node;

    lir_t *val = query_ident(sema, name);

    if (val != NULL) {
        return lir_name(node, lir_type(val), val);
    }

    message_t *id = report(sema->reports, ERROR, node, "unknown variable name `%s`", name);
    report_note(id, "PL/0 is case insensitive");

    return lir_poison(node, "unresolved variable");
}

static lir_t *compile_binary(sema_t *sema, pl0_t *expr) {
    lir_t *lhs = compile_expr(sema, expr->lhs);
    lir_t *rhs = compile_expr(sema, expr->rhs);

    const type_t *type = types_common(lir_type(lhs), lir_type(rhs));

    return lir_binary(expr->node, type, expr->binary, lhs, rhs);
}

static lir_t *compile_unary(sema_t *sema, pl0_t *expr) {
    lir_t *operand = compile_expr(sema, expr->operand);

    return lir_unary(expr->node, lir_type(operand), expr->unary, operand);
}

static lir_t *compile_odd(sema_t *sema, pl0_t *expr) {
    lir_t *operand = compile_expr(sema, expr->operand);

    node_t *node = expr->node;
    lir_t *rem = lir_binary(node, lir_type(operand), BINARY_REM, operand, pl0_num(node, 2));
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
        ctu_assert(sema->reports, "(pl0) compile-expr unknown expr %d", expr->type);
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
        ctu_assert(sema->reports, "compile-cond unknown cond %d", expr->type);
        return lir_poison(expr->node, "unknown cond");
    }
}

static lir_t *compile_assign(sema_t *sema, pl0_t *stmt) {
    node_t *node = stmt->node;
    char *name = pl0_name(stmt->dst);
    lir_t *lhs = query_ident(sema, name);
    if (lhs == NULL) {
        message_t *id = report(sema->reports, ERROR, node, "unknown variable name `%s`", name);
        report_note(id, "PL/0 is case insensitive");

        return lir_poison(node, "unresolved variable");
    }

    lir_t *rhs = compile_expr(sema, stmt->src);

    if (is_const(lhs->type)) {
        report(sema->reports, ERROR, node, "cannot assign to const value `%s`", name);
    }

    return lir_assign(node, lhs, rhs);
}

static lir_t *compile_print(sema_t *sema, pl0_t *stmt) {
    lir_t *expr = compile_expr(sema, stmt->operand);

    lir_t *print = sema_get_data(sema);

    vector_t *args = vector_new(2);
    vector_push(&args, lir_string(stmt->node, type_string(), "%d\n"));
    vector_push(&args, expr);

    return lir_call(stmt->node, closure_result(lir_type(print)), print, args);
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
        report(sema->reports, ERROR, stmt->node, "unknown procedure `%s`", name);
        return lir_poison(stmt->node, "unknown procedure");
    }

    return lir_call(stmt->node, closure_result(lir_type(proc)), proc, vector_new(0));
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
        ctu_assert(sema->reports, "compile-stmt unknown stmt %d", stmt->type);
        return lir_poison(stmt->node, "unknown stmt");
    }
}

static void compile_const(sema_t *sema, lir_t *lir) {
    pl0_t *node = lir->ctx;
    lir_t *value = compile_expr(sema, node->value);

    vector_t *path = lir_recurses(value, lir);
    if (path != NULL) {
        report_recursive(sema->reports, path, lir);
    }
    const type_t *type = pl0_int(false);

    lir_value(sema->reports, lir, 
        /* type = */ type,
        /* init = */ value
    );
}

static lir_t *build_var(sema_t *sema, pl0_t *node) {
    char *name = pl0_name(node->name);
    lir_t *lir = lir_forward(node->node, name, LIR_VALUE, NULL);
    const type_t *type = pl0_int(true);
    lir_t *init = pl0_num(node->node, 0);

    lir_value(sema->reports, lir, 
        /* type = */ type, 
        /* init = */ init
    );
    
    sema_set(sema, TAG_VARS, name, lir);

    return lir;
}

static void compile_var(sema_t *sema, lir_t *lir) {
    pl0_t *node = lir->ctx;
    lir_t *value = pl0_num(node->node, 0);
    type_t *type = pl0_int(true);

    lir_value(sema->reports, lir, type, value);
}

static void compile_proc(sema_t *sema, lir_t *lir) {
    pl0_t *node = lir->ctx;
    size_t nlocals = vector_len(node->locals);
    vector_t *locals = vector_of(nlocals);

    size_t sizes[TAG_MAX] = {
        [TAG_VARS] = nlocals,
        [TAG_CONSTS] = 0,
        [TAG_PROCS] = 0,
    };
    sema_t *nest = NEW_SEMA(sema, sema->path, sema->reports, sizes);
    sema_set_data(nest, sema_get_data(sema));

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

    lir_define(sema->reports, lir, 
        /* type = */ pl0_closure(), 
        /* locals = */ locals,
        /* body = */ lir_stmts(node->node, body)
    );

    DELETE_SEMA(nest);
}

static lir_t *pl0_declare(pl0_t *pl0, const char *name, leaf_t leaf) {
    return lir_forward(pl0->node, name, leaf, pl0);
}

static const attrib_t ENTRY = {
    .visibility = ENTRYPOINT,
    .mangle = "main"
};

static lir_t *compile_entry(sema_t *sema, pl0_t *body) {    
    lir_t *stmts = compile_stmt(sema, body);

    lir_t *entry = pl0_declare(body, "main", LIR_DEFINE);

    lir_define(sema->reports, entry,
        /* type = */ pl0_closure(),
        /* locals = */ vector_of(0),
        /* body = */ lir_stmts(body->node, vector_init(stmts))
    );

    lir_attribs(entry, &ENTRY);

    return entry;
}

lir_t *pl0_sema(reports_t *reports, pl0_t *node) {
    vector_t *vars = node->globals;
    vector_t *consts = node->consts;
    vector_t *procs = node->procs;

    size_t nvars = vector_len(vars);
    size_t nconsts = vector_len(consts);
    size_t nprocs = vector_len(procs);

    size_t sizes[TAG_MAX] = {
        [TAG_VARS] = nvars,
        [TAG_CONSTS] = nconsts,
        [TAG_PROCS] = nprocs
    };
    sema_t *sema = NEW_SEMA(NULL, NODE_PATH(node), reports, sizes);
    lir_t *print = pl0_import_print(reports, node->node);
    sema_set_data(sema, print);

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

    map_t *const_map = sema_tag(sema, TAG_CONSTS);
    map_t *var_map = sema_tag(sema, TAG_VARS);
    map_t *proc_map = sema_tag(sema, TAG_PROCS);

    MAP_APPLY(const_map, sema, compile_const);
    MAP_APPLY(var_map, sema, compile_var);
    MAP_APPLY(proc_map, sema, compile_proc);

    vector_t *globals = vector_join(
        map_values(var_map),
        map_values(const_map)
    );

    vector_t *funcs = map_values(proc_map);

    if (node->toplevel) {
        vector_push(&funcs, compile_entry(sema, node->toplevel));
    }

    DELETE_SEMA(sema);

    return lir_module(node->node,
        /* imports = */ vector_init(print),
        globals, 
        funcs
    );
}
