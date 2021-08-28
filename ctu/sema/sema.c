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

static bool is_ordered(sema_t *sema) {
    return !sema->options->order_independent;
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

static void report_redefine(sema_t *sema, const char *id, node_t *self, node_t *other) {
    report_t err = reportf(ERROR, self->scan, self->where, "value `%s` already defined", id);
    report_append(err, other->scan, other->where, "previously defined here");

    if (is_nocase(sema)) {
        report_note(err, "%s is case insensitive", self->scan->language);
    }
}
 
static void add_value(sema_t *sema, const char *id, lir_t *value) {
    lir_t *other = get_value(sema, id);
    if (other) {
        report_redefine(sema, id, value->node, other->node);
    }
    
    map_set(sema->values, id, value);
}

static lir_t *get_define(sema_t *sema, const char *id) {
    lir_t *define = map_get(sema->defines, id);
    if (define) {
        return define;
    }

    if (sema->parent != NULL) {
        return get_define(sema->parent, id);
    }

    return NULL;
}

static void add_define(sema_t *sema, const char *id, lir_t *define) {
    lir_t *other = get_define(sema, id);

    if (other) {
        report_redefine(sema, id, define->node, other->node);
    }

    map_set(sema->defines, id, define);
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

    case AST_DEFINE:
        add_define(sema, name, lir);
        break;

    default:
        assert("unimplemented declare-decl %d", decl->kind);
        break;
    }
}

static type_t *compile_value(sema_t *sema, const char *id, lir_t *value) {

}

static type_t *compile_define(sema_t *sema, const char *id, lir_t *define) {

}

static void declare_all(sema_t *sema, vector_t *decls) {
    size_t len = vector_len(decls);

    /* forward declare all declarations */
    for (size_t i = 0; i < len; i++) {
        node_t *decl = vector_get(decls, i);
        declare_decl(sema, decl);
    }

    map_apply(sema->values, sema, (map_apply_t)compile_value);
    map_apply(sema->defines, sema, (map_apply_t)compile_define);
}

static void compile_all(sema_t *sema, vector_t *decls) {
    (void)sema;
    (void)decls;
}

lir_t *sema_module(node_t *node) {
    if (node->kind != AST_MODULE) {
        assert("sema-module only works on modules");
        return NULL;
    }

    sema_t *sema = sema_new(node->options, NULL);

    vector_t *decls = node->decls;

    /** 
     * either forward everything and compile for languages like pl0
     * or compile everything in order for languages like C
     */
    if (is_ordered(sema)) {
        compile_all(sema, decls);
    } else {
        declare_all(sema, decls);
    }

    sema_delete(sema);

    return NULL;
}
