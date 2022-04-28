#include "sema.h"
#include "ast.h"

#include "cthulhu/hlir/decl.h"
#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/type.h"

#include "cthulhu/util/report-ext.h"
#include "cthulhu/util/set.h"

typedef enum {
    TAG_VARS,    // hlir_t*
    TAG_PROCS,   // hlir_t*
    TAG_TYPES,   // hlir_t*
    TAG_MODULES, // sema_t*

    TAG_MAX
} tag_t;

static bool is_discard_ident(const char *id) {
    return id == NULL || str_equal(id, "$");
}

static hlir_t *sema_type(sema_t *sema, ast_t *ast);

static hlir_t *sema_typename(sema_t *sema, ast_t *ast) {
    size_t len = vector_len(ast->path);
    if (len > 1) {
        ctu_assert(
            sema->reports,
            "typename path can only be 1 element long currently");
        return hlir_error(
            ast->node, "typename path can only be 1 element long currently");
    }

    const char *name = vector_tail(ast->path);
    hlir_t *decl = sema_get(sema, TAG_TYPES, name);
    if (decl == NULL) {
        report(sema->reports, ERROR, ast->node, "type '%s' not found", name);
        return hlir_error(ast->node, "type not found");
    }

    return decl;
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
    hlir_t *result = sema_type(sema, ast->result);
    size_t len = vector_len(ast->params);
    vector_t *params = vector_of(len);

    for (size_t i = 0; i < len; i++) {
        ast_t *param = vector_get(ast->params, i);
        hlir_t *type = sema_type(sema, param);
        vector_set(params, i, type);
    }

    return hlir_closure(ast->node, NULL, params, result, ast->variadic);
}

static hlir_t *sema_type(sema_t *sema, ast_t *ast) {
    switch (ast->of) {
    case AST_TYPENAME:
        return sema_typename(sema, ast);
    case AST_POINTER:
        return sema_pointer(sema, ast);
    case AST_ARRAY:
        return sema_array(sema, ast);
    case AST_CLOSURE:
        return sema_closure(sema, ast);
    default:
        ctu_assert(sema->reports, "unknown sema-type: %d", ast->of);
        return hlir_error(ast->node, "unknown sema-type");
    }
}

static void
check_duplicates_and_add_fields(sema_t *sema, vector_t *fields, hlir_t *decl) {
    size_t len = vector_len(fields);
    set_t *names = set_new(len);

    for (size_t i = 0; i < len; i++) {
        ast_t *field = vector_get(fields, i);
        const char *name = field->name;

        if (!is_discard_ident(name)) {
            if (set_contains(names, name)) {
                report(
                    sema->reports,
                    ERROR,
                    field->node,
                    "field '%s' already defined",
                    name);
                continue;
            }

            (void)set_add(names, name);
        }

        hlir_t *type = sema_type(sema, field->field);
        hlir_t *entry = hlir_field(field->node, type, name);
        hlir_add_field(decl, entry);
    }

    set_delete(names);
}

static void sema_struct(sema_t *sema, hlir_t *decl, ast_t *ast) {
    vector_t *fields = ast->fields;

    check_duplicates_and_add_fields(sema, fields, decl);

    hlir_build_struct(decl);
}

static void sema_union(sema_t *sema, hlir_t *decl, ast_t *ast) {
    vector_t *fields = ast->fields;

    check_duplicates_and_add_fields(sema, fields, decl);

    hlir_build_union(decl);
}

static void sema_alias(sema_t *sema, hlir_t *decl, ast_t *ast) {
    hlir_t *type = sema_type(sema, ast->alias);
    hlir_build_alias(decl, type, false);
}

static void sema_variant(sema_t *sema, hlir_t *decl, ast_t *ast) {
    report(sema->reports, INTERNAL, ast->node, "variant not implemented");
    hlir_t *tag = hlir_digit(
        ast->node, format("%s_tag", ast->name), DIGIT_INT, SIGN_UNSIGNED);
    hlir_t *field = hlir_field(ast->node, tag, "tag");
    hlir_add_field(decl, field);

    hlir_build_struct(decl);
}

static void sema_decl(sema_t *sema, ast_t *ast) {
    hlir_t *decl;

    switch (ast->of) {
    case AST_STRUCTDECL:
        decl = sema_get(sema, TAG_TYPES, ast->name);
        sema_struct(sema, decl, ast);
        break;

    case AST_UNIONDECL:
        decl = sema_get(sema, TAG_TYPES, ast->name);
        sema_union(sema, decl, ast);
        break;

    case AST_ALIASDECL:
        decl = sema_get(sema, TAG_TYPES, ast->name);
        sema_alias(sema, decl, ast);
        break;

    case AST_VARIANTDECL:
        decl = sema_get(sema, TAG_TYPES, ast->name);
        sema_variant(sema, decl, ast);
        break;

    default:
        ctu_assert(sema->reports, "unexpected ast of type %d", ast->of);
        break;
    }
}

static void add_decl(sema_t *sema, tag_t tag, const char *name, hlir_t *decl) {
    if (is_discard_ident(name)) {
        report(sema->reports, ERROR, decl->node, "discarding declaration");
        return;
    }

    hlir_t *other = sema_get(sema, tag, name);
    if (other != NULL) {
        report_shadow(sema->reports, name, decl->node, other->node);
        return;
    }

    sema_set(sema, tag, name, decl);
}

static void fwd_decl(sema_t *sema, ast_t *ast) {
    hlir_t *decl;

    switch (ast->of) {
    case AST_STRUCTDECL:
        decl = hlir_begin_struct(ast->node, ast->name);
        break;

    case AST_UNIONDECL:
        decl = hlir_begin_union(ast->node, ast->name);
        break;

    case AST_ALIASDECL:
        decl = hlir_begin_alias(ast->node, ast->name);
        break;

    case AST_VARIANTDECL:
        decl = hlir_begin_struct(ast->node, ast->name);
        break;

    default:
        ctu_assert(sema->reports, "unexpected ast of type %d", ast->of);
        return;
    }

    add_decl(sema, TAG_TYPES, ast->name, decl);
}

static void add_basic_types(sema_t *sema) {
    const node_t *node = node_builtin();

    add_decl(sema, TAG_TYPES, "bool", hlir_bool(node, "bool"));
    add_decl(sema, TAG_TYPES, "str", hlir_string(node, "str"));

    add_decl(
        sema,
        TAG_TYPES,
        "char",
        hlir_digit(node, "char", DIGIT_CHAR, SIGN_SIGNED));
    add_decl(
        sema,
        TAG_TYPES,
        "uchar",
        hlir_digit(node, "uchar", DIGIT_CHAR, SIGN_UNSIGNED));

    add_decl(
        sema,
        TAG_TYPES,
        "short",
        hlir_digit(node, "short", DIGIT_SHORT, SIGN_SIGNED));
    add_decl(
        sema,
        TAG_TYPES,
        "ushort",
        hlir_digit(node, "ushort", DIGIT_SHORT, SIGN_UNSIGNED));

    add_decl(
        sema,
        TAG_TYPES,
        "int",
        hlir_digit(node, "int", DIGIT_INT, SIGN_SIGNED));
    add_decl(
        sema,
        TAG_TYPES,
        "uint",
        hlir_digit(node, "uint", DIGIT_INT, SIGN_UNSIGNED));

    add_decl(
        sema,
        TAG_TYPES,
        "long",
        hlir_digit(node, "long", DIGIT_LONG, SIGN_SIGNED));
    add_decl(
        sema,
        TAG_TYPES,
        "ulong",
        hlir_digit(node, "ulong", DIGIT_LONG, SIGN_UNSIGNED));

    // enable the below later

    // special types for interfacing with C
    // sysv says that enums are signed ints
    // add_decl(sema, TAG_TYPES, "enum", hlir_digit(node, "enum", DIGIT_INT,
    // SIGN_SIGNED));
}

hlir_t *ctu_sema(runtime_t *runtime, void *ast) {
    ast_t *root = ast;

    size_t ndecls = vector_len(root->decls);

    size_t sizes[] = { [TAG_VARS] = ndecls,
                       [TAG_PROCS] = ndecls,
                       [TAG_TYPES] = ndecls,
                       [TAG_MODULES] = ndecls };

    sema_t *sema = sema_new(NULL, runtime->reports, TAG_MAX, sizes);

    add_basic_types(sema);

    for (size_t i = 0; i < ndecls; i++) {
        fwd_decl(sema, vector_get(root->decls, i));
    }

    for (size_t i = 0; i < ndecls; i++) {
        sema_decl(sema, vector_get(root->decls, i));
    }

    vector_t *values = map_values(sema_tag(sema, TAG_VARS));
    vector_t *types = map_values(sema_tag(sema, TAG_TYPES));
    hlir_t *mod = hlir_module(root->node, "todo", types, values, vector_of(0));

    sema_delete(sema);

    return mod;
}
