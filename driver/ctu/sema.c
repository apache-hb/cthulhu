#include "sema.h"
#include "ast.h"

#include "cthulhu/hlir/hlir.h"

typedef enum {
    TAG_VARS,
    TAG_MAX
} tag_t;

static void sema_var(sema_t *sema, ast_t *ast) {
    char *name = ast->name;
    att_t *type = ast->type;
    ast_t *init = ast->init;

    
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
        [TAG_VARS] = ndecls
    };

    sema_t *sema = sema_new(NULL, reports, TAG_MAX, sizes);

    for (size_t i = 0; i < ndecls; i++) {
        sema_decl(sema, vector_get(root->decls, i));
    }

    sema_delete(sema);
}
