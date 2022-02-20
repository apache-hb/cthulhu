#include "sema.h"
#include "ast.h"

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/util/report-ext.h"

typedef enum {
    TAG_VARS, // hlir_t*
    TAG_PROCS, // hlir_t*
    TAG_TYPES, // hlir_t*
    TAG_MODULES, // sema_t*

    TAG_MAX
} tag_t;

static tag_t TAGOF[] = {
    [AST_TYPEALIAS] = TAG_TYPES,
    [AST_STRUCTDECL] = TAG_TYPES,
    [AST_UNIONDECL] = TAG_TYPES
};

static hlir_t *sema_type(sema_t *sema, ast_t *ast);

static hlir_t *sema_typename(sema_t *sema, ast_t *ast) {
    UNUSED(sema);

    return hlir_error(ast->node, "typename not unimplemented");
}

static hlir_t *sema_pointer(sema_t *sema, ast_t *ast) {
    hlir_t *type = sema_type(sema, ast->type);
    return hlir_pointer(ast->node, NULL, type, ast->indexable);
}

static hlir_t *sema_array(sema_t *sema, ast_t *ast) {
    UNUSED(sema);

    return hlir_error(ast->node, "array not implemented");
}

static hlir_t *sema_closure(sema_t *sema, ast_t *ast) {
    UNUSED(sema);

    return hlir_error(ast->node, "closure not implemented");
}

static hlir_t *sema_type(sema_t *sema, ast_t *ast) {
    switch (ast->of) {
    case AST_TYPENAME: return sema_typename(sema, ast);
    case AST_POINTER: return sema_pointer(sema, ast);
    case AST_ARRAY: return sema_array(sema, ast);
    case AST_CLOSURE: return sema_closure(sema, ast);
    default:
        ctu_assert(sema->reports, "unknown sema-type: %d", ast->of);
        return hlir_error(ast->node, "unknown sema-type");
    }
}

static void sema_alias(sema_t *sema, hlir_t *decl, ast_t *ast) {
    UNUSED(sema);
    UNUSED(decl);
    UNUSED(ast);
}

static void sema_struct(sema_t *sema, hlir_t *decl, ast_t *ast) {
    UNUSED(sema);
    UNUSED(decl);
    UNUSED(ast);
}

static void sema_union(sema_t *sema, hlir_t *decl, ast_t *ast) {
    UNUSED(sema);
    UNUSED(decl);
    UNUSED(ast);
}

static void sema_decl(sema_t *sema, ast_t *ast) {
    hlir_t *decl = sema_get(sema, TAGOF[ast->of], ast->name);
    if (decl->type != HLIR_FORWARD) {
        report(sema->reports, ERROR, ast->node, "redeclaration of `%s`", ast->name);
        return;
    }

    switch (ast->of) {
    case AST_TYPEALIAS:
        sema_alias(sema, decl, ast);
        break;

    case AST_STRUCTDECL:
        sema_struct(sema, decl, ast);
        break;

    case AST_UNIONDECL:
        sema_union(sema, decl, ast);
        break;

    default:
        ctu_assert(sema->reports, "unexpected ast of type %d", ast->of);
    }
}

static void add_decl(sema_t *sema, tag_t tag, const char *name, hlir_t *decl) {
    hlir_t *other = sema_get(sema, tag, name);
    if (other != NULL) {
        report_shadow(sema->reports, name, decl->node, other->node);
        return;
    }

    sema_set(sema, tag, name, decl);
}

static hlir_t *fwd_decl(sema_t *sema, ast_t *ast) {
    hlir_t *decl;
    switch (ast->of) {
    case AST_STRUCTDECL:
        decl = hlir_new_struct(ast->node, ast->name);
        add_decl(sema, TAG_TYPES, ast->name, decl);
        break;

    case AST_UNIONDECL:
        decl = hlir_new_union(ast->node, ast->name);
        add_decl(sema, TAG_TYPES, ast->name, decl);
        break;

    case AST_TYPEALIAS:
        decl = hlir_new_alias(ast->node, ast->name);
        add_decl(sema, TAG_TYPES, ast->name, decl);
        break;
    
    default:
        ctu_assert(sema->reports, "unexpected ast of type %d", ast->of);
        return hlir_error(ast->node, "unexpected ast of type");
    }

    return decl;
}

static void add_basic_types(sema_t *sema) {
    sema_set(sema, TAG_TYPES, "bool", hlir_bool(NULL, "bool"));

    sema_set(sema, TAG_TYPES, "char", hlir_digit(NULL, "char", DIGIT_CHAR, SIGN_SIGNED));
    sema_set(sema, TAG_TYPES, "uchar", hlir_digit(NULL, "uchar", DIGIT_CHAR, SIGN_UNSIGNED));

    sema_set(sema, TAG_TYPES, "short", hlir_digit(NULL, "short", DIGIT_SHORT, SIGN_SIGNED));
    sema_set(sema, TAG_TYPES, "ushort", hlir_digit(NULL, "ushort", DIGIT_SHORT, SIGN_UNSIGNED));

    sema_set(sema, TAG_TYPES, "int", hlir_digit(NULL, "int", DIGIT_INT, SIGN_SIGNED));
    sema_set(sema, TAG_TYPES, "uint", hlir_digit(NULL, "uint", DIGIT_INT, SIGN_UNSIGNED));

    sema_set(sema, TAG_TYPES, "long", hlir_digit(NULL, "long", DIGIT_LONG, SIGN_SIGNED));
    sema_set(sema, TAG_TYPES, "ulong", hlir_digit(NULL, "ulong", DIGIT_LONG, SIGN_UNSIGNED));
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

    add_basic_types(sema);
    
    for (size_t i = 0; i < ndecls; i++) {
        fwd_decl(sema, vector_get(root->decls, i));
    }

    for (size_t i = 0; i < ndecls; i++) {
        sema_decl(sema, vector_get(root->decls, i));
    }

    sema_delete(sema);

    hlir_t *mod = hlir_new_module(root->node, "todo");
    hlir_build_module(mod, vector_of(0), vector_of(0), vector_of(0));

    return mod;
}
