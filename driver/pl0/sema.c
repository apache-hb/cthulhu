#include "sema.h"

#include "cthulhu/hlir/sema.h"
#include "cthulhu/util/report-ext.h"

typedef enum {
    TAG_CONSTS,
    TAG_VARS,
    TAG_PROCS,

    TAG_MAX
} pl0_tag_t;

static type_t *INT;
static type_t *BOOL;
static type_t *VOID;
static type_t *STRING;
static hlir_t *PRINT;

void pl0_init() {
    scan_t *scan = scan_builtin("PL/0");
    node_t *node = node_builtin(scan);

    INT = type_integer(node, "integer", WIDTH_INT, SIGN_SIGNED);
    BOOL = type_boolean(node, "boolean");
    VOID = type_void(node, "void");
    STRING = type_string(node, "string");

    // printf :: (string, ...) -> int
    type_t *closure = type_closure(node, "printf", vector_init(STRING), INT, true);

    PRINT = hlir_function(node, closure, "printf", NULL);
}

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

    hlir_t *var_hlir = sema_get(sema, TAG_VARS, name);
    if (var_hlir != NULL) { return var_hlir; }

    return NULL;
}

static void set_proc(sema_t *sema, const char *name, hlir_t *proc) {
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

static hlir_t *sema_binary(sema_t *sema, pl0_t *node) {
    hlir_t *lhs = sema_expr(sema, node->lhs);
    hlir_t *rhs = sema_expr(sema, node->rhs);

    return hlir_binary(node->node, INT, lhs, rhs, node->binary);
}

static hlir_t *sema_name(sema_t *sema, pl0_t *node) {
    hlir_t *hlir = get_var(sema, node->ident);
    if (hlir == NULL) {
        report_pl0_unresolved(sema->reports, node->node, node->ident);
        return hlir_error(node->node, NULL);
    }

    return hlir_name(node->node, INT, hlir);
}

static hlir_t *sema_expr(sema_t *sema, pl0_t *node) {
    switch (node->type) {
    case PL0_DIGIT: return hlir_digit(node->node, INT, node->digit);
    case PL0_BINARY: return sema_binary(sema, node);
    case PL0_IDENT: return sema_name(sema, node);
    default: 
        report(sema->reports, INTERNAL, node->node, "sema-expr");
        return NULL;
    }
}

static hlir_t *sema_assign(sema_t *sema, pl0_t *node) {
    hlir_t *dst = get_var(sema, node->dst);
    if (dst == NULL) {
        report_pl0_unresolved(sema->reports, node->node, node->dst);
        return hlir_error(node->node, NULL);
    }

    hlir_t *src = sema_expr(sema, node->src);

    return hlir_assign(node->node, dst, src);
}

static hlir_t *sema_print(sema_t *sema, pl0_t *node) {
    hlir_t *fmt = hlir_string(node->node, STRING, "%d\n");
    hlir_t *hlir = sema_expr(sema, node->print);

    vector_t *args = vector_new(2);
    vector_push(&args, fmt);
    vector_push(&args, hlir);

    return hlir_call(node->node, INT, PRINT, args);
}

static hlir_t *sema_odd(sema_t *sema, pl0_t *node) {
    hlir_t *operand = sema_expr(sema, node->operand);

    // `# expr` in pl0 is `expr % 2 != 0` in hlir 
    hlir_t *two = hlir_int(node->node, INT, 2);
    hlir_t *zero = hlir_int(node->node, INT, 0);
    hlir_t *mod = hlir_binary(node->node, INT, operand, two, BINARY_REM);
    hlir_t *eq = hlir_compare(node->node, BOOL, mod, zero, COMPARE_NEQ);

    return eq;
}

static hlir_t *sema_compare(sema_t *sema, pl0_t *node) {
    hlir_t *lhs = sema_expr(sema, node->lhs);
    hlir_t *rhs = sema_expr(sema, node->rhs);

    return hlir_compare(node->node, BOOL, lhs, rhs, node->compare);
}
 
static hlir_t *sema_call(sema_t *sema, pl0_t *node) {
    hlir_t *proc = get_proc(sema, node->procedure);
    if (proc == NULL) {
        report_pl0_unresolved(sema->reports, node->node, node->procedure);
        return hlir_error(node->node, NULL);
    }

    return hlir_call(node->node, VOID, proc, vector_of(0));
}

static hlir_t *sema_cond(sema_t *sema, pl0_t *node) {
    switch (node->type) {
    case PL0_ODD: return sema_odd(sema, node);
    case PL0_COMPARE: return sema_compare(sema, node);
    default:
        report(sema->reports, INTERNAL, node->node, "sema-cond");
        return NULL;
    }
}

static hlir_t *sema_stmt(sema_t *sema, pl0_t *node);

static hlir_t *sema_stmts(sema_t *sema, pl0_t *node) {
    vector_t *stmts = node->stmts;

    size_t len = vector_len(stmts);
    vector_t *result = vector_of(len);

    for (size_t i = 0; i < len; i++) {
        pl0_t *stmt = vector_get(stmts, i);
        hlir_t *hlir = sema_stmt(sema, stmt);
        vector_set(result, i, hlir);
    }

    return hlir_stmts(node->node, result);
}

static hlir_t *sema_loop(sema_t *sema, pl0_t *node) {
    hlir_t *cond = sema_cond(sema, node->cond);
    hlir_t *body = sema_stmt(sema, node->then);

    return hlir_while(node->node, cond, body);
}

static hlir_t *sema_branch(sema_t *sema, pl0_t *node) {
    hlir_t *cond = sema_cond(sema, node->cond);
    hlir_t *then = sema_stmt(sema, node->then);

    return hlir_branch(node->node, cond, then, NULL);
}

static hlir_t *sema_stmt(sema_t *sema, pl0_t *node) {
    switch (node->type) {
    case PL0_ASSIGN: return sema_assign(sema, node);
    case PL0_PRINT: return sema_print(sema, node);
    case PL0_LOOP: return sema_loop(sema, node);
    case PL0_CALL: return sema_call(sema, node);
    case PL0_BRANCH: return sema_branch(sema, node);
    case PL0_STMTS: return sema_stmts(sema, node);
    default:
        report(sema->reports, INTERNAL, node->node, "sema-stmt");
        return NULL;
    }
}

static hlir_t *sema_value(sema_t *sema, pl0_t *node) {
    hlir_t *init = node->value != NULL 
        ? sema_expr(sema, node->value) 
        : hlir_int(node->node, INT, 0);
    return hlir_value(node->node, NULL, node->name, init);
}

static hlir_t *sema_proc(sema_t *sema, pl0_t *node) {
    size_t num_locals = vector_len(node->locals);

    size_t sizes[TAG_MAX] = {
        [TAG_CONSTS] = 0,
        [TAG_VARS] = num_locals,
        [TAG_PROCS] = 0
    };

    sema_t *nest = sema_new(sema, sema->reports, TAG_MAX, sizes);

    vector_t *stmts = vector_of(num_locals);
    for (size_t i = 0; i < num_locals; i++) {
        pl0_t *local = vector_get(node->locals, i);
        hlir_t *hlir = sema_value(nest, local);
        set_var(sema, TAG_VARS, local->name, hlir);
        vector_set(stmts, i, hlir);
    }

    for (size_t i = 0; i < vector_len(node->body); i++) {
        pl0_t *stmt = vector_get(node->body, i);
        hlir_t *hlir = sema_stmt(nest, stmt);
        vector_push(&stmts, hlir);
    }

    sema_delete(nest);

    type_t *signature = type_closure(node->node, format("%s-signature", node->name), vector_of(0), VOID, false);
    return hlir_function(node->node, signature, node->name, stmts);
}

hlir_t *pl0_sema(reports_t *reports, void *node) {
    pl0_t *root = node;

    size_t num_consts = vector_len(root->consts);
    size_t num_globals = vector_len(root->globals);
    size_t num_procs = vector_len(root->procs);

    vector_t *consts_decls = vector_of(num_consts);
    vector_t *globals_decls = vector_of(num_globals);
    vector_t *procs_decls = vector_of(num_procs);

    size_t sizes[TAG_MAX] = {
        [TAG_CONSTS] = num_consts,
        [TAG_VARS] = num_globals,
        [TAG_PROCS] = num_procs
    };

    sema_t *sema = sema_new(NULL, reports, TAG_MAX, sizes);

    // forward declare all our declarations
    for (size_t i = 0; i < num_consts; i++) {
        pl0_t *it = vector_get(root->consts, i);
        hlir_t *hlir = hlir_declare(it->node, it->name, HLIR_VALUE);
        set_var(sema, TAG_CONSTS, it->name, hlir);
        vector_set(consts_decls, i, hlir);
    }

    for (size_t i = 0; i < num_globals; i++) {
        pl0_t *it = vector_get(root->globals, i);
        hlir_t *hlir = hlir_declare(it->node, it->name, HLIR_VALUE);
        set_var(sema, TAG_VARS, it->name, hlir);
        vector_set(globals_decls, i, hlir);
    }

    for (size_t i = 0; i < num_procs; i++) {
        pl0_t *it = vector_get(root->procs, i);
        hlir_t *hlir = hlir_declare(it->node, it->name, HLIR_FUNCTION);
        set_proc(sema, it->name, hlir);
        vector_set(procs_decls, i, hlir);
    }

    // now compile all our declarations
    for (size_t i = 0; i < num_consts; i++) {
        pl0_t *it = vector_get(root->consts, i);
        hlir_t *hlir = vector_get(consts_decls, i);
        hlir_t *temp = sema_value(sema, it);
        *hlir = *temp;
    }

    for (size_t i = 0; i < num_globals; i++) {
        pl0_t *it = vector_get(root->globals, i);
        hlir_t *hlir = vector_get(globals_decls, i);
        hlir_t *temp = sema_value(sema, it);
        *hlir = *temp;
    }

    for (size_t i = 0; i < num_procs; i++) {
        pl0_t *it = vector_get(root->procs, i);
        hlir_t *hlir = vector_get(procs_decls, i);
        hlir_t *temp = sema_proc(sema, it);
        *hlir = *temp;
    }

    vector_t *globals = vector_join(
        map_values(sema_tag(sema, TAG_VARS)), 
        map_values(sema_tag(sema, TAG_CONSTS))
    );

    vector_t *procs = map_values(sema_tag(sema, TAG_PROCS));
    vector_t *imports = vector_init(PRINT);

    sema_delete(sema);

    return hlir_module(root->node, root->mod, imports, globals, procs);
}
