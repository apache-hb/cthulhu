#include "sema.h"

#include "ctu/util/report.h"

#include <ctype.h>

typedef struct {
    map_t *vars;
    map_t *consts;
    map_t *procs;
} pl0_data_t;

static void *pl0_data_new(void) {
    pl0_data_t *sema = ctu_malloc(sizeof(pl0_data_t));
    sema->vars = map_new(4);
    sema->consts = map_new(4);
    sema->procs = map_new(4);
    return sema;
}

static void pl0_data_delete(void *data) {
    pl0_data_t *sema = data;
    map_delete(sema->vars);
    map_delete(sema->consts);
    map_delete(sema->procs);
    ctu_free(sema);
}

#define NEW_SEMA(parent) sema_new(parent, pl0_data_new)
#define DELETE_SEMA(sema) sema_delete(sema, pl0_data_delete)

static void report_shadow(const char *name, node_t *other, node_t *self) {
    report_t id = reportf(ERROR, self->scan, self->where, "refinition of `%s`", name);
    report_append(id, other->scan, other->where, "previous definition");
    report_note(id, "PL/0 is case insensitive");
}

#define PL0_ADD(field, get, func) \
    static void func(sema_t *sema, const char *name, lir_t *lir) { \
        pl0_data_t *data = sema->fields; \
        lir_t *other = get(sema, name); \
        if (other != NULL) { \
            report_shadow(name, other->node, lir->node); \
        } \
        map_set(data->field, name, lir); \
    }

#define PL0_ADD2(field, get1, get2, func) \
    static void func(sema_t *sema, const char *name, lir_t *lir) { \
        pl0_data_t *data = sema->fields; \
        lir_t *other1 = get1(sema, name); \
        if (other1 != NULL) { \
            report_shadow(name, other1->node, lir->node); \
        } \
        lir_t *other2 = get2(sema, name); \
        if (other2 != NULL) { \
            report_shadow(name, other2->node, lir->node); \
        } \
        map_set(data->field, name, lir); \
    }

#define PL0_GET(field, func) \
    static lir_t *func(sema_t *sema, const char *name) { \
        pl0_data_t *data = sema->fields; \
        return map_get(data->field, name); \
    }

PL0_GET(vars, pl0_get_var)
PL0_GET(consts, pl0_get_const)
PL0_GET(procs, pl0_get_proc)

PL0_ADD2(vars, pl0_get_var, pl0_get_const, pl0_add_var)
PL0_ADD2(consts, pl0_get_const, pl0_get_var, pl0_add_const)
PL0_ADD(procs, pl0_get_proc, pl0_add_proc)

#if 0

#include "ctu/util/report.h"

#include <ctype.h>

static type_t *DIGIT = NULL;
static type_t *RESULT = NULL;

#define NEW_SEMA(parent) sema_new(parent, pl0_data_new)
#define DELETE_SEMA(sema) sema_delete(sema, pl0_data_delete)
#define GET_GLOBAL(sema, name) sema_get(sema, name, pl0_get_global)
#define SET_GLOBAL(sema, name, value) sema_set(sema, name, value, pl0_set_global)

#define GET_PROC(sema, name) sema_get(sema, name, pl0_get_proc)
#define SET_PROC(sema, name, proc) sema_set(sema, name, proc, pl0_set_proc)

static void report_shadow(const char *name, node_t *other, node_t *self) {
    report_t id = reportf(ERROR, self->scan, self->where, "refinition of `%s`", name);
    report_append(id, other->scan, other->where, "previous definition");
    report_note(id, "PL/0 is case insensitive");
}

static lir_t *pl0_get_global(sema_t *sema, const char *name) {
    pl0_data_t *data = sema->fields;

    return map_get(data->globals, name);
}

static void pl0_set_global(sema_t *sema, const char *name, lir_t *value) {
    lir_t *other = GET_GLOBAL(sema, name);
    if (other != NULL) {
        report_shadow(name, other->node, value->node);
        return;
    }
    
    pl0_data_t *data = sema->fields;
    map_set(data->globals, name, value);
}

static lir_t *pl0_get_proc(sema_t *sema, const char *name) {
    pl0_data_t *data = sema->fields;
    return map_get(data->procs, name);
}

static void pl0_set_proc(sema_t *sema, const char *name, lir_t *proc) {
    lir_t *other = GET_PROC(sema, name);
    if (other != NULL) {
        report_shadow(name, other->node, proc->node);
        return;
    }
    
    pl0_data_t *data = sema->fields;
    map_set(data->procs, name, proc);
}

static leaf_t decl_leaf(pl0_t *decl) {
    switch (decl->kind) {
    case AST_VALUE: return LIR_VALUE;
    case AST_DEFINE: return LIR_DEFINE;

    default:
        assert("unknown decl-leaf %d", decl->kind);
        return LIR_EMPTY;
    }
}

static char *pl0_name(node_t *ident) {
    char *out = ctu_strdup(ident->ident);
    for (char *p = out; *p != '\0'; p++) {
        *p = tolower(*p);
    }
    return out;
}

static void pl0_declare(sema_t *sema, node_t *decl) {
    leaf_t leaf = decl_leaf(decl);
    char *name = pl0_name(decl->name);
    lir_t *lir = lir_declare(decl, name, leaf, NULL);

    switch (leaf) {
    case LIR_VALUE:
        SET_GLOBAL(sema, name, lir);
        break;
    case LIR_DEFINE:
        SET_PROC(sema, name, lir);
        break;
    default:
        assert("unknown leaf %d", leaf);
        break;
    }
}

static lir_t *pl0_compile_expr(sema_t *sema, node_t *expr);
static lir_t *pl0_compile_stmt(sema_t *sema, node_t *stmt);

static lir_t *pl0_compile_binary(sema_t *sema, node_t *expr) {
    lir_t *lhs = pl0_compile_expr(sema, expr->lhs);
    lir_t *rhs = pl0_compile_expr(sema, expr->rhs);

    return lir_binary(expr, expr->binary, lhs, rhs);
}

static lir_t *pl0_compile_unary(sema_t *sema, node_t *expr) {
    lir_t *operand = pl0_compile_expr(sema, expr->operand);

    return lir_unary(expr, expr->unary, operand);
}

static lir_t *pl0_compile_digit(node_t *expr) {
    return lir_digit(expr, expr->digit);
}

static lir_t *pl0_match_expr(sema_t *sema, pl0_t *expr) {
    switch (expr->kind) {
    case AST_IDENT:
        return GET_GLOBAL(sema, expr->ident);
    case AST_DIGIT:
        return pl0_compile_digit(expr);
    case AST_BINARY:
        return pl0_compile_binary(sema, expr);
    case AST_UNARY:
        return pl0_compile_unary(sema, expr);

    default:
        assert("unknown expr %d", expr->kind);
        return lir_poison(expr->node, "unknown expr");
    }
}

static lir_t *pl0_compile_expr(sema_t *sema, pl0_t *expr) {
    lir_t *lir = pl0_match_expr(sema, expr);
    lir->type = DIGIT;
    return lir;
}

static void report_recurse(vector_t *stack, lir_t *root) {
    node_t *node = root->node;
    report_t id = reportf(ERROR, node->scan, node->where, "initialization of `%s` is recursive", root->name);
    
    node_t *last = node;
    size_t len = vector_len(stack);
    size_t t = 0;

    for (size_t i = 0; i < len; i++) {
        lir_t *lir = vector_get(stack, i);
        node_t *it = lir->node;
        if (it != last) {
            report_append(id, it->scan, it->where, "trace %zu", t++);
        }
        last = it;
    }
}

static lir_t *pl0_compile_assign(sema_t *sema, node_t *expr) {
    lir_t *dst = pl0_compile_expr(sema, expr->dst);
    lir_t *src = pl0_compile_expr(sema, expr->src);

    return lir_assign(expr, dst, src);
}

static void pl0_compile_local(sema_t *sema, node_t *expr) {
    lir_t *init = lir_int(expr, 0);
    init->type = DIGIT;

    const char *name = pl0_name(expr->name);

    lir_t *lir = lir_declare(expr, name, LIR_VALUE, NULL);
    lir_begin(lir, LIR_VALUE);
    lir_value(lir, DIGIT, init);

    SET_GLOBAL(sema, name, lir);
}

static lir_t *pl0_compile_stmts(sema_t *sema, node_t *stmts) {
    return NULL; /* TODO */
}

static lir_t *pl0_compile_stmt(sema_t *sema, node_t *stmt) {
    switch (stmt->kind) {
    case AST_ASSIGN:
        return pl0_compile_assign(sema, stmt);

    case AST_VALUE:
        pl0_compile_local(sema, stmt);
        return NULL;

    case AST_STMTS:
        return pl0_compile_stmts(sema, stmt);

    default:
        return NULL;
    }
}

static void pl0_global(void *user, lir_t *lir) {
    lir_begin(lir, LIR_VALUE);

    sema_t *sema = user;

    node_t *node = lir->node;
    lir_t *init = node->value
        ? pl0_compile_expr(sema, node->value) 
        : lir_int(node, 0);
    
    init->type = DIGIT;

    vector_t *path = lir_recurses(init, lir);
    if (path != NULL) {
        report_recurse(path, lir);
    }

    lir_value(lir, DIGIT, init);
}

static void pl0_proc(void *user, lir_t *lir) {
    lir_begin(lir, LIR_DEFINE);
    
    sema_t *sema = user;
    node_t *node = lir->node;

    sema_t *nest = NEW_SEMA(sema);

    lir_t *proc = pl0_compile_stmt(nest, node->body);
}

static bool always(void *value) {
    return value != NULL;
}

static lir_t *pl0_compile(sema_t *sema, pl0_t *root) {
    /* declare everything */
    vector_t *decls = root->decls;
    size_t len = vector_len(decls);
    for (size_t i = 0; i < len; i++) {
        pl0_t *decl = vector_get(decls, i);
        pl0_declare(sema, decl);
    }

    /* compile everything */
    pl0_data_t *data = sema->fields;

    MAP_APPLY(data->globals, sema, pl0_global);
    MAP_APPLY(data->procs, sema, pl0_proc);

    vector_t *vars = MAP_COLLECT(data->globals, always);
    vector_t *procs = MAP_COLLECT(data->procs, always);

    return lir_module(root, vars, procs);
}

lir_t *pl0_sema(pl0_t *node) {
    sema_t *sema = NEW_SEMA(NULL);

    if (DIGIT == NULL) {
        DIGIT = type_digit(true, TY_INT);
    }

    if (RESULT == NULL) {
        RESULT = type_closure(vector_new(0), DIGIT);
    }

    lir_t *result = pl0_compile(sema, node);

    DELETE_SEMA(sema);

    return result;
}
#endif

static char *pl0_name(const char *name) {
    char *out = ctu_strdup(name);
    for (char *p = out; *p != '\0'; p++) {
        *p = tolower(*p);
    }
    return out;
}

static lir_t *pl0_declare(pl0_t *pl0, leaf_t leaf) {
    return lir_forward(pl0->node, pl0_name(pl0->name), leaf, NULL);
}

static bool always(void *value) {
    return value != NULL;
}

lir_t *pl0_sema(pl0_t *node) {
    sema_t *sema = NEW_SEMA(NULL);

    vector_t *vars = node->globals;
    vector_t *consts = node->consts;
    vector_t *procs = node->procs;

    size_t nvars = vector_len(vars);
    size_t nconsts = vector_len(consts);
    size_t nprocs = vector_len(procs);

    for (size_t i = 0; i < nvars; i++) {
        pl0_t *decl = vector_get(vars, i);
        pl0_add_var(sema, decl->name, pl0_declare(decl, LIR_VALUE));
    }

    for (size_t i = 0; i < nconsts; i++) {
        pl0_t *decl = vector_get(consts, i);
        pl0_add_const(sema, decl->name, pl0_declare(decl, LIR_VALUE));
    }

    for (size_t i = 0; i < nprocs; i++) {
        pl0_t *decl = vector_get(procs, i);
        pl0_add_proc(sema, decl->name, pl0_declare(decl, LIR_DEFINE));
    }

    pl0_data_t *data = sema->fields;

    vector_t *globals = vector_join(
        MAP_COLLECT(data->vars, always),
        MAP_COLLECT(data->consts, always)
    );

    vector_t *funcs = MAP_COLLECT(data->procs, always);

    DELETE_SEMA(sema);

    return lir_module(node->node, globals, funcs);
}
