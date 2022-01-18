#include "sema.h"

#include "cthulhu/hlir/sema.h"
#include "cthulhu/util/report-ext.h"

typedef enum {
    TAG_CONSTS,
    TAG_VARS,
    TAG_PROCS,

    TAG_MAX
} pl0_tag_t;

static void report_pl0_shadowing(reports_t *reports, const char *name, node_t *shadowed, node_t *shadowing) {
    message_t *id = report_shadow(reports, name, shadowed, shadowing);
    report_note(id, "PL/0 is case insensitive");
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
    hlir_t *other1 = sema_get(sema, TAG_CONSTS, name);
    if (other1 != NULL) {
        report_pl0_shadowing(sema->reports, name, other1->node, hlir->node);
        return;
    }

    hlir_t *other2 = sema_get(sema, TAG_VARS, name);
    if (other2 != NULL) {
        report_pl0_shadowing(sema->reports, name, other2->node, hlir->node);
        return;
    }

    sema_set(sema, tag, name, hlir);
}

hlir_t *pl0_sema(reports_t *reports, void *node) {
    pl0_t *root = node;

    size_t num_consts = vector_len(root->consts);
    size_t num_globals = vector_len(root->globals);
    size_t num_procs = vector_len(root->procs);

    size_t sizes[TAG_MAX] = {
        [TAG_CONSTS] = num_consts,
        [TAG_VARS] = num_globals,
        [TAG_PROCS] = num_procs
    };

    sema_t *sema = sema_new(NULL, reports, TAG_MAX, sizes);

    for (size_t i = 0; i < num_consts; i++) {
        pl0_t *it = vector_get(root->consts, i);
        set_var(sema, TAG_CONSTS, it->name, hlir_declare(it->node, it->name, HLIR_VALUE));
    }

    for (size_t i = 0; i < num_globals; i++) {
        pl0_t *it = vector_get(root->globals, i);
        set_var(sema, TAG_VARS, it->name, hlir_declare(it->node, it->name, HLIR_VALUE));
    }

    for (size_t i = 0; i < num_procs; i++) {
        pl0_t *it = vector_get(root->procs, i);
        set_proc(sema, it->name, hlir_declare(it->node, it->name, HLIR_FUNCTION));
    }

    sema_delete(sema);

    return hlir_module(root->node, root->mod, vector_of(0), vector_of(0), vector_of(0));
}
