#include "sema.h"

#include "ctu/util/report.h"
#include "ctu/util/str.h"

#if 0

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

static lir_t *get_value(sema_t *sema, node_t *node, const char *id) {
    lir_t *value = map_get(sema->values, id);
    if (value) {
        return value;
    }

    if (sema->parent != NULL) {
        return get_value(sema->parent, node, id);
    }

    return lir_poison(node, "unresolved value");
}

static void report_redefine(sema_t *sema, const char *id, node_t *self, node_t *other) {
    report_t err = reportf(ERROR, self->scan, self->where, "value `%s` already defined", id);
    report_append(err, other->scan, other->where, "previously defined here");

    if (is_nocase(sema)) {
        report_note(err, "%s is case insensitive", self->scan->language);
    }
}

static void add_value(sema_t *sema, const char *id, lir_t *value) {
    lir_t *other = get_value(sema, value->node, id);
    if (lir_ok(other)) {
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

    if (lir_ok(other)) {
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

    lir_t *lir = lir_declare(decl, name, leaf, sema);

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

static type_t *resolve_typename(sema_t *sema, node_t *it) {
    node_t *ident = it->id;
    const char *name = normalize(sema, ident->ident);
    
    map_t *builtins = sema->options->types;
    type_t *builtin = map_get(builtins, name);
    if (builtin) {
        return builtin;
    }

    reportf(ERROR, ident->scan, ident->where, "failed to resolve type `%s`", name);
    return NULL;
}

static type_t *resolve_type(sema_t *sema, node_t *type) {
    switch (type->kind) {
    case AST_TYPE: return type->builtin;
    case AST_TYPENAME: return resolve_typename(sema, type);

    default: 
        assert("resolve-type unimplemented %d", type->kind);
        return NULL;
    }
}

static lir_t *compile_digit(node_t *digit) {
    lir_t *lir = lir_digit(digit, digit->digit);
    lir_resolve(lir, type_digit(true, TY_INT));
    return lir;
}

static lir_t *compile_value(sema_t *sema, lir_t *value);

static lir_t *build_value(lir_t *lir) {
    if (!lir_is(lir, LIR_EMPTY)) {
        if (lir->type == NULL) {
            node_t *node = lir->node;
            reportf(ERROR, node->scan, node->where, "recursive resolution");
            return lir_poison(node, "recursive resolution");
        }
        
        return lir;
    }

    return compile_value(lir->sema, lir);
}

static lir_t *compile_ident(sema_t *sema, node_t *ident) {
    lir_t *value = get_value(sema, ident, ident->ident);
    if (value) {
        return build_value(value);
    }

    lir_t *define = get_define(sema, ident->ident);
    if (define) {
        return define;
    }

    reportf(ERROR, ident->scan, ident->where, "undefined value `%s`", ident->ident);
    return lir_poison(ident, ctu_strdup("unresolved expression"));
}

static lir_t *compile_expr(sema_t *sema, node_t *expr) {
    switch (expr->kind) {
    case AST_DIGIT: return compile_digit(expr);
    case AST_IDENT: return compile_ident(sema, expr);
    default: return NULL;
    }
}

static lir_t *compile_value(sema_t *sema, lir_t *value) {
    lir_begin(value, LIR_VALUE);

    node_t *node = value->node;
    type_t *type = NULL;
    lir_t *init = NULL;

    if (node->type) {
        type = resolve_type(sema, node->type);
    }

    if (node->value) {
        init = compile_expr(sema, node->value);
    }

    if (type == NULL) {
        type = lir_resolved(init);
    }

    lir_value(value, type, init);

    return value;
}

static type_t *compile_define(sema_t *sema, lir_t *define) {
    (void)sema;
    (void)define;
    return NULL;
}

static void apply_value(void *user, void *value) {
    if (lir_is(value, LIR_EMPTY)) {
        compile_value(user, value);
    }
}

static void apply_define(void *user, void *define) {
    compile_define(user, define);
}

static void declare_all(sema_t *sema, vector_t *decls) {
    size_t len = vector_len(decls);

    /* forward declare all declarations */
    for (size_t i = 0; i < len; i++) {
        node_t *decl = vector_get(decls, i);
        declare_decl(sema, decl);
    }

    map_apply(sema->values, sema, apply_value);
    map_apply(sema->defines, sema, apply_define);
}

static void compile_all(sema_t *sema, vector_t *decls) {
    (void)sema;
    (void)decls;
}

static bool yes(void *value) {
    return value != NULL;
}

static lir_t *build_module(sema_t *sema, node_t *node) {
    vector_t *vars = map_collect(sema->values, yes);
    vector_t *defs = map_collect(sema->defines, yes);
    return lir_module(node, vars, defs);
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

    lir_t *mod = build_module(sema, node);

    sema_delete(sema);

    return mod;
}

#endif

sema_t *sema_new(sema_t *parent, sema_new_t create) {
    sema_t *sema = ctu_malloc(sizeof(sema_t));
    
    sema->parent = parent;
    sema->fields = create();

    return sema;
}

void sema_delete(sema_t *sema, sema_delete_t destroy) {
    destroy(sema->fields);
}

void sema_set(sema_t *sema, const char *name, lir_t *lir, sema_set_t set) {
    set(sema, name, lir);
}

lir_t *sema_get(sema_t *sema, const char *name, sema_get_t get) {
    lir_t *lir = get(sema, name);
    if (lir != NULL) {
        return lir;
    }

    if (sema->parent != NULL) {
        return sema_get(sema->parent, name, get);
    }

    return NULL;
}
