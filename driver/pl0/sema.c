#include "sema.h"

#include "cthulhu/hlir/sema.h"
#include "cthulhu/util/report-ext.h"

static hlir_t *INTEGER;
static hlir_t *BOOLEAN;
static hlir_t *STRING;
static hlir_t *VOID;

static hlir_t *PRINT;
static hlir_t *FMT;

static const hlir_attributes_t *IMPORTED;
static const hlir_attributes_t *EXPORTED;

void pl0_init(void) {
    const node_t *node = node_builtin();

    IMPORTED = hlir_new_attributes(LINK_IMPORTED);
    EXPORTED = hlir_new_attributes(LINK_EXPORTED);

    INTEGER = hlir_digit(node, "integer", DIGIT_INT, SIGN_DEFAULT);
    BOOLEAN = hlir_bool(node, "boolean");
    STRING = hlir_string(node, "string");
    VOID = hlir_void(node, "void");

    FMT = hlir_string_literal(node, STRING, "%d\n");
    PRINT = hlir_new_function(node, "printf", vector_init(STRING), INTEGER, true);
    hlir_set_attributes(PRINT, IMPORTED);
    hlir_build_function(PRINT, NULL);
}

typedef enum {
    TAG_VALUES,
    TAG_CONSTS,
    TAG_PROCS,

    TAG_MAX
} tag_t;

static void report_pl0_shadowing(reports_t *reports, const char *name, const node_t *shadowed, const node_t *shadowing) {
    message_t *id = report_shadow(reports, name, shadowed, shadowing);
    report_note(id, "PL/0 is case insensitive");
}

static void report_pl0_unresolved(reports_t *reports, const node_t *node, const char *name) {
    report(reports, ERROR, node, "unresolved reference to `%s`", name);
}

static hlir_t *get_var(sema_t *sema, const char *name) {
    hlir_t *const_hlir = sema_get(sema, TAG_CONSTS, name);
    if (const_hlir != NULL) { return const_hlir; }

    hlir_t *var_hlir = sema_get(sema, TAG_VALUES, name);
    if (var_hlir != NULL) { return var_hlir; }

    return NULL;
}

static void set_proc(sema_t *sema, const char *name, hlir_t *proc) {
    /* kinda hacky but works well enough */
    if (streq(name, "main")) {
        report(sema->reports, ERROR, proc->node, "main is a reserved name");
        return;
    }

    hlir_t *other = sema_get(sema, TAG_PROCS, name);
    if (other != NULL) {
        report_pl0_shadowing(sema->reports, name, other->node, proc->node);
        return;
    }

    sema_set(sema, TAG_PROCS, name, proc);
}

static hlir_t *get_proc(sema_t *sema, const char *name) {
    return sema_get(sema, TAG_PROCS, name);
}

static void set_var(sema_t *sema, size_t tag, const char *name, hlir_t *hlir) {
    hlir_t *other = get_var(sema, name);
    if (other != NULL) {
        report_pl0_shadowing(sema->reports, name, other->node, hlir->node);
        return;
    }

    sema_set(sema, tag, name, hlir);
}

static hlir_t *sema_expr(sema_t *sema, pl0_t *node);
static hlir_t *sema_compare(sema_t *sema, pl0_t *node);
static hlir_t *sema_stmt(sema_t *sema, pl0_t *node);

static hlir_t *sema_digit(pl0_t *node) {
    return hlir_digit_literal(node->node, INTEGER, node->digit);
}

static hlir_t *sema_ident(sema_t *sema, pl0_t *node) {
    hlir_t *var = get_var(sema, node->ident);
    if (var == NULL) {
        report_pl0_unresolved(sema->reports, node->node, node->ident);
        return hlir_error(node->node, "unresolved identifier");
    }
    return hlir_name(node->node, var);
}

static hlir_t *sema_binary(sema_t *sema, pl0_t *node) {
    hlir_t *lhs = sema_expr(sema, node->lhs);
    hlir_t *rhs = sema_expr(sema, node->rhs);
    return hlir_binary(node->node, INTEGER, node->binary, lhs, rhs);
}

static hlir_t *sema_expr(sema_t *sema, pl0_t *node) {
    switch (node->type) {
    case PL0_DIGIT: return sema_digit(node);
    case PL0_IDENT: return sema_ident(sema, node);
    case PL0_BINARY: return sema_binary(sema, node);
    default:
        report(sema->reports, INTERNAL, node->node, "sema-expr: %d", node->type);
        return hlir_error(node->node, "sema-expr");
    }
}

static hlir_t *sema_vector(sema_t *sema, const node_t *node, vector_t *body) {
    size_t len = vector_len(body);
    vector_t *result = vector_of(len);

    for (size_t i = 0; i < len; i++) {
        pl0_t *it = vector_get(body, i);
        hlir_t *temp = sema_stmt(sema, it);
        vector_set(result, i, temp);
    }

    return hlir_stmts(node, result);
}

static hlir_t *sema_stmts(sema_t *sema, pl0_t *node) {
    return sema_vector(sema, node->node, node->stmts);
}

static hlir_t *sema_call(sema_t *sema, pl0_t *node) {
    hlir_t *proc = get_proc(sema, node->procedure);
    if (proc == NULL) {
        report_pl0_unresolved(sema->reports, node->node, node->procedure);
        return hlir_error(node->node, "unresolved procedure");
    }

    vector_t *args = vector_new(0);

    return hlir_call(node->node, proc, args);
}

static hlir_t *sema_branch(sema_t *sema, pl0_t *node) {
    hlir_t *cond = sema_compare(sema, node->cond);
    hlir_t *then = sema_stmt(sema, node->then);

    return hlir_branch(node->node, cond, then, NULL);
}

static hlir_t *sema_assign(sema_t *sema, pl0_t *node) {
    hlir_t *dst = get_var(sema, node->dst);
    hlir_t *src = sema_expr(sema, node->src);

    return hlir_assign(node->node, dst, src);
}

static hlir_t *sema_loop(sema_t *sema, pl0_t *node) {
    hlir_t *cond = sema_compare(sema, node->cond);
    hlir_t *body = sema_stmt(sema, node->then);

    return hlir_loop(node->node, cond, body, NULL);
}

static hlir_t *sema_print(sema_t *sema, pl0_t *node) {
    hlir_t *expr = sema_expr(sema, node->print);
    
    vector_t *args = vector_of(2);
    vector_set(args, 0, FMT);
    vector_set(args, 1, expr);

    return hlir_call(node->node, PRINT, args);
}

static hlir_t *sema_stmt(sema_t *sema, pl0_t *node) {
    switch (node->type) {
    case PL0_STMTS: return sema_stmts(sema, node);
    case PL0_CALL: return sema_call(sema, node);
    case PL0_BRANCH: return sema_branch(sema, node);
    case PL0_LOOP: return sema_loop(sema, node);
    case PL0_ASSIGN: return sema_assign(sema, node);
    case PL0_PRINT: return sema_print(sema, node);
    default:
        report(sema->reports, INTERNAL, node->node, "sema-stmt: %d", node->type);
        return hlir_error(node->node, "sema-stmt");
    }
}

static hlir_t *sema_value(sema_t *sema, pl0_t *node) {
    pl0_t *val = node->value;
    if (val == NULL) {
        return hlir_int_literal(node->node, INTEGER, 0);
    } else {
        return sema_expr(sema, val);
    }
}

static hlir_t *sema_odd(sema_t *sema, pl0_t *node) {
    hlir_t *val = sema_expr(sema, node->operand);
    hlir_t *two = hlir_int_literal(node->node, INTEGER, 2);
    hlir_t *one = hlir_int_literal(node->node, INTEGER, 1);
    hlir_t *rem = hlir_binary(node->node, INTEGER, BINARY_REM, val, two);
    hlir_t *eq = hlir_compare(node->node, BOOLEAN, COMPARE_EQ, rem, one);

    return eq;
}

static hlir_t *sema_comp(sema_t *sema, pl0_t *node) {
    hlir_t *lhs = sema_expr(sema, node->lhs);
    hlir_t *rhs = sema_expr(sema, node->rhs);

    return hlir_compare(node->node, BOOLEAN, node->compare, lhs, rhs);
}

static hlir_t *sema_compare(sema_t *sema, pl0_t *node) {
    switch (node->type) {
    case PL0_ODD: return sema_odd(sema, node);
    case PL0_COMPARE: return sema_comp(sema, node);
    default:
        report(sema->reports, INTERNAL, node->node, "sema-compare: %d", node->type);
        return hlir_error(node->node, "sema-compare");
    }
}

static void sema_proc(sema_t *sema, hlir_t *hlir, pl0_t *node) {
    size_t nlocals = vector_len(node->locals);
    size_t sizes[TAG_MAX] = {
        [TAG_VALUES] = nlocals
    };

    sema_t *nest = sema_new(sema, sema->reports, TAG_MAX, sizes);

    for (size_t i = 0; i < nlocals; i++) {
        pl0_t *local = vector_get(node->locals, i);
        hlir_t *it = hlir_value(local->node, local->name, INTEGER, NULL);
        hlir_add_local(hlir, it);
        set_var(nest, TAG_VALUES, local->name, it);
    }

    hlir_t *body = sema_vector(nest, node->node, node->body);

    sema_delete(nest);

    hlir_build_function(hlir, body);
}

hlir_t *pl0_sema(reports_t *reports, void *node) {
    pl0_t *root = node;

    size_t nconsts = vector_len(root->consts);
    size_t nglobals = vector_len(root->globals);
    size_t nprocs = vector_len(root->procs);

    vector_t *consts = vector_of(nconsts);
    vector_t *globals = vector_of(nglobals);
    vector_t *procs = vector_of(nprocs);

    size_t sizes[TAG_MAX] = {
        [TAG_CONSTS] = nconsts,
        [TAG_VALUES] = nglobals,
        [TAG_PROCS] = nprocs
    };

    sema_t *sema = sema_new(NULL, reports, TAG_MAX, sizes);
    
    // forward declare everything

    for (size_t i = 0; i < nconsts; i++) {
        pl0_t *it = vector_get(root->consts, i);
        hlir_t *hlir = hlir_new_value(it->node, it->name, INTEGER);
        set_var(sema, TAG_CONSTS, it->name, hlir);
        vector_set(consts, i, hlir);
    }

    for (size_t i = 0; i < nglobals; i++) {
        pl0_t *it = vector_get(root->globals, i);
        hlir_t *hlir = hlir_new_value(it->node, it->name, INTEGER);
        set_var(sema, TAG_VALUES, it->name, hlir);
        vector_set(globals, i, hlir);
    }

    for (size_t i = 0; i < nprocs; i++) {
        pl0_t *it = vector_get(root->procs, i);
        hlir_t *hlir = hlir_new_function(it->node, it->name, vector_of(0), VOID, false);
        set_proc(sema, it->name, hlir);
        vector_set(procs, i, hlir);
    }

    // compile everything

    for (size_t i = 0; i < nconsts; i++) {
        pl0_t *it = vector_get(root->consts, i);
        hlir_t *hlir = vector_get(consts, i);
        hlir_build_value(hlir, sema_value(sema, it));
    }

    for (size_t i = 0; i < nglobals; i++) {
        pl0_t *it = vector_get(root->globals, i);
        hlir_t *hlir = vector_get(globals, i);
        hlir_build_value(hlir, sema_value(sema, it));
    }

    for (size_t i = 0; i < nprocs; i++) {
        pl0_t *it = vector_get(root->procs, i);
        hlir_t *hlir = vector_get(procs, i);
        sema_proc(sema, hlir, it);
    }

    if (root->entry) {
        hlir_t *hlir = hlir_new_function(root->node, "main", vector_of(0), VOID, false);
        hlir_t *body = sema_stmt(sema, root->entry);
        hlir_build_function(hlir, body);
        hlir_set_attributes(hlir, EXPORTED);
        vector_push(&procs, hlir);
    }

    vector_push(&procs, PRINT);

    vector_t *types = vector_new(5);
    vector_push(&types, INTEGER);
    vector_push(&types, BOOLEAN);
    vector_push(&types, STRING);
    vector_push(&types, VOID);

    hlir_t *mod = hlir_new_module(root->node, root->mod);
    hlir_build_module(mod, vector_join(consts, globals), procs, types);
    sema_delete(sema);
    return mod;
}
