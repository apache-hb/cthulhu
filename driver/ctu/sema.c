#include "sema.h"
#include "ast.h"

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/util/report-ext.h"

typedef enum {
    TAG_VARS, // hlir_t*
    TAG_PROCS, // hlir_t*
    TAG_TYPES, // type_t*
    TAG_MODULES, // sema_t*

    TAG_MAX
} tag_t;

static sema_t *resolve_path(sema_t *root, const node_t *node, vector_t *path) {
    size_t len = vector_len(path);
    sema_t *sema = root;
    
    for (size_t i = 0; i < len - 1; i++) {
        const char *name = vector_get(path, i);
        sema = sema_get(sema, TAG_MODULES, name);
        if (sema == NULL) {
            report(root->reports, ERROR, node, "failed to resolve path segment %zu `%s`", i, name);
            return NULL;
        }
    }

    return sema;
}

static hlir_t *get_decl(sema_t *sema, const char *ident) {
    hlir_t *var = sema_get(sema, TAG_VARS, ident);
    if (var != NULL) { return var; }

    hlir_t *proc = sema_get(sema, TAG_PROCS, ident);
    if (proc != NULL) { return proc; }

    return NULL;
}

static hlir_t *sema_name(sema_t *sema, ast_t *ast) {
    vector_t *path = ast->path;
    const char *ident = vector_tail(path);

    sema_t *root = resolve_path(sema, ast->node, path);
    if (root == NULL) { return NULL; }

    hlir_t *decl = get_decl(root, ident);
    if (decl == NULL) {
        report(root->reports, ERROR, ast->node, "failed to resolve name `%s`", ident);
        return NULL;
    }

    return hlir_name(ast->node, decl);
}

static hlir_t *sema_digit(sema_t *sema, ast_t *ast) {
    UNUSED(sema);

    type_t *type = type_integer("int-literal");
    value_t *digit = value_digit(type, ast->digit);

    return hlir_literal(ast->node, digit);
}

static hlir_t *sema_expr(sema_t *sema, ast_t *ast) {
    switch (ast->of) {
    case AST_NAME: return sema_name(sema, ast);
    case AST_DIGIT: return sema_digit(sema, ast);
    }
}

static void sema_var(sema_t *sema, ast_t *ast) {
    const char *name = ast->name;
    att_t *type = ast->type;
    ast_t *init = ast->init;

    hlir_t *other = get_decl(sema, name);
    if (other != NULL) {
        report_shadow(sema->reports, name, other->node, ast->node);
        return;
    }

    hlir_t *hlir = sema_expr(sema, init);

    hlir_t *decl = hlir_value(ast->node, name, typeof_hlir(hlir), hlir);

    sema_set(sema, TAG_VARS, name, decl);
}

static void sema_decl(sema_t *sema, ast_t *ast) {
    switch (ast->of) {
    case AST_VAR: 
        sema_var(sema, ast); 
        break;
    
    default:
        ctu_assert(sema->reports, "unexpected ast of type %d", ast->of);
    }
}

hlir_t *ctu_sema(reports_t *reports, void *ast) {
    ast_t *root = ast;

    size_t ndecls = vector_len(root->decls);

    size_t sizes[] = {
        [TAG_VARS] = ndecls,
        [TAG_PROCS] = ndecls,
        [TAG_TYPES] = ndecls,
        [TAG_MODULES] = ndecls
    };

    sema_t *sema = sema_new(NULL, reports, TAG_MAX, sizes);

    for (size_t i = 0; i < ndecls; i++) {
        sema_decl(sema, vector_get(root->decls, i));
    }

    sema_delete(sema);
}
