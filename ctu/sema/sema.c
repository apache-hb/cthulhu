#include "sema.h"

#include "ctu/util/report.h"
#include "ctu/util/str.h"

#include <ctype.h>

typedef struct sema_t {
    const options_t *options;
    struct sema_t *parent;

    /* map_t<const char*, lir_t*> */
    map_t *values;

    /* map_t<const char*, lir_t*> */
    map_t *defines;

    /* map_t<const char*, type_t*> */
    map_t *types;
} sema_t;

static sema_t *sema_new(const options_t *options, sema_t *parent) {
    sema_t *sema = ctu_malloc(sizeof(sema_t));
    sema->options = options;
    sema->parent = parent;
    
    sema->values = map_new(8);
    sema->defines = map_new(8);
    sema->types = map_new(8);

    return sema;
}

static void sema_delete(sema_t *sema) {
    map_delete(sema->values);
    map_delete(sema->defines);
    map_delete(sema->types);
    ctu_free(sema);
}

static bool is_nocase(sema_t *sema) {
    return !sema->options->case_sensitive;
}

static const char *normalize(sema_t *sema, const char *name) {
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

static lir_t *get_value(sema_t *sema, const char *id) {
    lir_t *value = map_get(sema->values, id);
    if (value) {
        return value;
    }

    if (sema->parent != NULL) {
        return get_value(sema->parent, id);
    }

    return NULL;
}

static void add_value(sema_t *sema, const char *id, lir_t *value) {
    lir_t *other = get_value(sema, id);
    if (other) {
        node_t *node = other->node;
        node_t *self = value->node;

        report_t err = reportf(ERROR, self->scan, self->where, "value `%s` already defined", id);
        report_append(err, node->scan, node->where, "previously defined here");

        if (is_nocase(sema)) {
            report_note(err, "%s is case insensitive", self->scan->language);
        }
    }
    
    map_set(sema->values, id, value);
}

static leaf_t node_leaf(node_t *node) {
    switch (node->kind) {
    case AST_VALUE: return LIR_VALUE;
    case AST_DEFINE: return LIR_DEFINE;

    default: 
        assert("unknown node-leaf %d", node->kind);
        return LIR_EMPTY;
    }
}

static void declare_decl(sema_t *sema, node_t *decl) {
    leaf_t leaf = node_leaf(decl);
    const char *name = normalize(sema, decl->name->ident);

    lir_t *lir = lir_declare(decl, name, leaf);

    switch (decl->kind) {
    case AST_VALUE: 
        add_value(sema, name, lir); 
        break;

    default:
        assert("unimplemented declare-decl %d", decl->kind);
        break;
    }
}

lir_t *sema_module(node_t *node) {
    if (node->kind != AST_MODULE) {
        assert("sema-module only works on modules");
        return NULL;
    }

    sema_t *sema = sema_new(node->options, NULL);

    vector_t *decls = node->decls;
    size_t len = vector_len(decls);

    /* forward declare all declarations */
    /* TODO: only do this on languages that wants this */
    for (size_t i = 0; i < len; i++) {
        node_t *decl = vector_get(decls, i);
        declare_decl(sema, decl);
    }

    sema_delete(sema);

    return NULL;
}
