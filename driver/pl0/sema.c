#include "sema.h"

#include "cthulhu/hlir/sema.h"
#include "cthulhu/util/report-ext.h"

typedef enum {
    TAG_CONSTS,
    TAG_VARS,
    TAG_PROCS,

    TAG_MAX
} pl0_tag_t;

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

    return hlir_binary(node->node, NULL, lhs, rhs, node->binary);
}

static hlir_t *sema_name(sema_t *sema, pl0_t *node) {
    hlir_t *hlir = get_var(sema, node->ident);
    if (hlir == NULL) {
        report_pl0_unresolved(sema->reports, node->node, node->ident);
        return hlir_error(node->node, NULL);
    }

    // TODO: this will break eventually
    return hlir_name(node->node, NULL, hlir);
}

static hlir_t *sema_expr(sema_t *sema, pl0_t *node) {
    switch (node->type) {
    case PL0_DIGIT: return hlir_digit(node->node, NULL, node->digit);
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

static hlir_t *sema_stmt(sema_t *sema, pl0_t *node) {
    switch (node->type) {
    case PL0_ASSIGN: return sema_assign(sema, node);
    default:
        report(sema->reports, INTERNAL, node->node, "sema-stmt");
        return NULL;
    }
}

static hlir_t *sema_value(sema_t *sema, pl0_t *node) {
    hlir_t *init = node->value != NULL 
        ? sema_expr(sema, node->value) 
        : hlir_int(node->node, NULL, 0);
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

    vector_t *locals = vector_of(num_locals);
    for (size_t i = 0; i < num_locals; i++) {
        pl0_t *local = vector_get(node->locals, i);
        hlir_t *hlir = sema_value(nest, local);
        set_var(sema, TAG_VARS, local->name, hlir);
        vector_set(locals, i, hlir);
    }

    for (size_t i = 0; i < vector_len(node->body); i++) {
        pl0_t *stmt = vector_get(node->body, i);
        hlir_t *hlir = sema_stmt(nest, stmt);
        vector_push(&locals, hlir);
    }

    sema_delete(nest);

    // TODO: function signatures
    return hlir_function(node->node, NULL, node->name, locals);
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

    sema_delete(sema);

    return hlir_module(root->node, root->mod, vector_of(0), globals, procs);
}
