#include "sema.h"

#include "ctu/util/report.h"
#include "ctu/util/str.h"

#include <ctype.h>

typedef struct sema_t {
    const options_t *options;
    struct sema_t *parent;

    /** 
     * map_t<const char*, node_t*> 
     * 
     * map of all nodes in the current scope to their names.
     * 
     * if the language is case insensitive
     */
    map_t *decls;
} sema_t;

static sema_t *sema_new(const options_t *options, sema_t *parent) {
    sema_t *sema = ctu_malloc(sizeof(sema_t));
    sema->options = options;
    sema->parent = parent;
    sema->decls = map_new(8);
    return sema;
}

static bool is_nocase(sema_t *sema) {
    return !sema->options->case_sensitive;
}

static const char *normalize(sema_t *sema, node_t *ident) {
    const char *name = ident->ident;

    if (is_nocase(sema)) {
        char *out = ctu_strdup(name);
        char *temp = out;
        while (*temp) {
            char c = tolower(*temp);

            if (c == '_' || c == ' ') {
                c = '-';
            }

            *temp = c;

            temp++;
        }

        return out;
    }

    return name;
}

static const char *get_decl_name(sema_t *sema, node_t *node) {
    switch (node->kind) {
    case AST_VALUE:
    case AST_DEFINE:
        return normalize(sema, node->name);

    default:
        return NULL;
    }
}

static node_t *get_local_decl(sema_t *sema, const char *name) {
    return map_get(sema->decls, name);
}

static node_t *get_decl(sema_t *sema, node_t *node) {
    const char *name = get_decl_name(sema, node);
    if (name == NULL) {
        return NULL;
    }

    node_t *local = get_local_decl(sema, name);
    if (local != NULL) {
        return local;
    }

    if (sema->parent != NULL) {
        return get_decl(sema->parent, node);
    }

    return NULL;
}

static void add_unique_decl(sema_t *sema, node_t *node) {
    node_t *decl = get_decl(sema, node);
    if (decl != NULL) {
        report_t id = reportf(ERROR, node->scan, node->where, "redefinition of variable `%s`", get_decl_name(sema, node));
        report_append(id, decl->scan, decl->where, "previously defined here");
        if (is_nocase(sema)) {
            report_note(id, "%s is case insensitive", node->scan->language);
        }
        return;
    }

    const char *name = get_decl_name(sema, node);
    map_set(sema->decls, name, node);
}

static void enable_order_independence(sema_t *sema, node_t *node) {
    if (!sema->options->order_independent) {
        return;
    }

    size_t len = vector_len(node->declarations);
    for (size_t i = 0; i < len; i++) {
        node_t *decl = vector_get(node->declarations, i);
        add_unique_decl(sema, decl);
    }
}

lir_t *sema_module(node_t *node) {
    if (node->kind != AST_MODULE) {
        assert("sema-module only works on modules");
        return NULL;
    }

    const options_t *opts = node->options;

    sema_t *sema = sema_new(opts, NULL);

    enable_order_independence(sema, node);

    return lir_module(node, NULL, NULL);
}
