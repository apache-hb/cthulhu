#include "sema.h"

#include "ctu/util/report.h"

#include <ctype.h>

typedef struct {
    map_t *globals;
    map_t *procs;
} pl0_data_t;

static type_t *DIGIT = NULL;

static void *pl0_data_new(void) {
    pl0_data_t *sema = ctu_malloc(sizeof(pl0_data_t));
    sema->globals = map_new(4);
    sema->procs = map_new(4);
    return sema;
}

static void pl0_data_delete(void *data) {
    pl0_data_t *sema = data;
    map_delete(sema->globals);
    map_delete(sema->procs);
    ctu_free(sema);
}

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

static leaf_t decl_leaf(node_t *decl) {
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

static lir_t *pl0_compile_binary(sema_t *sema, node_t *expr) {
    lir_t *lhs = pl0_compile_expr(sema, expr->lhs);
    lir_t *rhs = pl0_compile_expr(sema, expr->rhs);

    return lir_binary(expr, expr->binary, lhs, rhs);
}

static lir_t *pl0_compile_expr(sema_t *sema, node_t *expr) {
    switch (expr->kind) {
    case AST_IDENT:
        return GET_GLOBAL(sema, expr->ident);
    case AST_DIGIT:
        return lir_digit(expr, expr->digit);
    case AST_BINARY:
        return pl0_compile_binary(sema, expr);

    default:
        assert("unknown expr %d", expr->kind);
        return lir_poison(expr, "unknown expr");
    }
}

static void report_recurse(vector_t *stack, lir_t *root) {
    node_t *node = root->node;
    report_t id = reportf(ERROR, node->scan, node->where, "initialization of `%s` is recursive", root->name);
    
    node_t *last = NULL;
    size_t len = vector_len(stack);
    for (size_t i = 0; i < len; i++) {
        lir_t *lir = vector_get(stack, i);
        node_t *it = lir->node;
        if (it != last) {
            report_append(id, it->scan, it->where, "relevant trace %zu/%zu", i, len - 1);
        }
        last = it;
    }
}

static void pl0_global(void *user, lir_t *lir) {
    lir_begin(lir, LIR_VALUE);

    sema_t *sema = user;

    node_t *node = lir->node;
    lir_t *init = node->value
        ? pl0_compile_expr(sema, node->value) 
        : lir_int(node, 0);
    
    vector_t *path = lir_recurses(init, lir);
    if (path != NULL) {
        report_recurse(path, lir);
    }

    lir_value(lir, DIGIT, init);
}

static void pl0_proc(void *user, lir_t *lir) {
    sema_t *sema = user;

    node_t *node = lir->node;
    (void)node;
    (void)sema;
}

static bool always(void *value) {
    return value != NULL;
}

static lir_t *pl0_compile(sema_t *sema, node_t *root) {
    /* declare everything */
    vector_t *decls = root->decls;
    size_t len = vector_len(decls);
    for (size_t i = 0; i < len; i++) {
        node_t *decl = vector_get(decls, i);
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

lir_t *pl0_sema(node_t *node) {
    sema_t *sema = NEW_SEMA(NULL);

    if (DIGIT == NULL) {
        DIGIT = type_digit(true, TY_INT);
    }

    lir_t *result = pl0_compile(sema, node);

    DELETE_SEMA(sema);

    return result;
}
