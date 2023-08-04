#include "ctu/sema.h"
#include "base/util.h"
#include "ctu/ast.h"

#include "cthulhu/mediator/driver.h"

#include "cthulhu/hlir/h2.h"
#include "cthulhu/hlir/query.h"

#include "report/report.h"
#include "report/report-ext.h"

#include "std/vector.h"
#include "std/map.h"
#include "std/str.h"

#include "base/memory.h"
#include "base/panic.h"
#include <ctype.h>
#include <string.h>

///
/// helpers
///

static bool is_discard(const char *name)
{
    return name == NULL;
}

///
/// get and set decls
///

static h2_t *get_decl(h2_t *sema, const char *name, const ctu_tag_t *tags, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        h2_t *decl = h2_module_get(sema, tags[i], name);
        if (decl != NULL) { return decl; }
    }

    return NULL;
}

static h2_t *get_namespace(h2_t *sema, const char *name)
{
    ctu_tag_t tags[] = { eTagModules, eTagImports };
    return get_decl(sema, name, tags, sizeof(tags) / sizeof(ctu_tag_t));
}

static h2_t *get_type(h2_t *sema, const char *name)
{
    ctu_tag_t tags[] = { eTagTypes };
    return get_decl(sema, name, tags, sizeof(tags) / sizeof(ctu_tag_t));
}

static void add_decl(h2_t *sema, ctu_tag_t tag, const char *name, h2_t *decl)
{
    CTASSERT(name != NULL);
    CTASSERT(decl != NULL);

    h2_t *old = h2_module_get(sema, tag, name);
    if (old != NULL)
    {
        report_shadow(sema->reports, name, h2_get_node(old), h2_get_node(decl));
    }
    else
    {
        h2_module_set(sema, tag, name, decl);
    }
}

///
/// init
///

static h2_t *kRootModule = NULL;
static h2_t *kIntTypes[eDigitTotal * eSignTotal] = { NULL };

static h2_t *make_int_type(const char *name, digit_t digit, sign_t sign)
{
    return (kIntTypes[digit * eSignTotal + sign] = h2_type_digit(node_builtin(), name, digit, sign));
}

static h2_t *get_int_type(digit_t digit, sign_t sign)
{
    return kIntTypes[digit * eSignTotal + sign];
}

static h2_t *make_runtime_mod(reports_t *reports)
{
    size_t sizes[eTagTotal] = {
        [eTagValues] = 1,
        [eTagTypes] = 1,
        [eTagFunctions] = 1,
        [eTagModules] = 1,
        [eTagImports] = 1,
        [eTagAttribs] = 1,
        [eTagSuffix] = 1,
    };

    node_t *node = node_builtin();
    h2_t *root = h2_module_root(reports, node, "runtime", eTagTotal, sizes);

    add_decl(root, eTagTypes, "int", make_int_type("int", eDigitInt, eSignSigned));
    add_decl(root, eTagTypes, "uint", make_int_type("uint", eDigitInt, eSignUnsigned));

    return root;
}

static vector_t *make_runtime_path(void)
{
    vector_t *path = vector_new(2);
    vector_push(&path, "ctu");
    vector_push(&path, "lang");
    return path;
}

void ctu_init(driver_t *handle)
{
    lifetime_t *lifetime = handle_get_lifetime(handle);
    reports_t *reports = lifetime_get_reports(lifetime);

    kRootModule = make_runtime_mod(reports);
    vector_t *path = make_runtime_path();

    context_t *ctx = compiled_new(handle, kRootModule);
    add_context(lifetime, path, ctx);
}

///
/// getting types
///

static h2_t *ctu_sema_type(h2_t *sema, const ctu_t *type);

static h2_t *ctu_sema_type_name(h2_t *sema, const ctu_t *type)
{
    size_t len = vector_len(type->typeName);
    h2_t *ns = sema;
    for (size_t i = 0; i < len - 1; i++)
    {
        const char *segment = vector_get(type->typeName, i);
        ns = get_namespace(ns, segment);
        if (ns == NULL)
        {
            report(sema->reports, eFatal, type->node, "namespace `%s` not found", segment);
            return h2_error(type->node, "namespace not found");
        }
    }

    const char *name = vector_tail(type->typeName);
    h2_t *decl = get_type(ns, name);
    if (decl == NULL)
    {
        report(sema->reports, eFatal, type->node, "type `%s` not found", name);
        return h2_error(type->node, "type not found");
    }

    return decl;
}

static h2_t *ctu_sema_type_pointer(h2_t *sema, const ctu_t *type)
{
    h2_t *pointee = ctu_sema_type(sema, type->pointer);
    return h2_type_pointer(type->node, pointee);
}

static h2_t *ctu_sema_type(h2_t *sema, const ctu_t *type)
{
    switch (type->kind)
    {
    case eCtuTypeName: return ctu_sema_type_name(sema, type);
    case eCtuTypePointer: return ctu_sema_type_pointer(sema, type);

    default: NEVER("invalid type kind %d", type->kind);
    }
}

///
/// expressions
///

static h2_t *ctu_sema_int(h2_t *sema, ctu_t *expr, const h2_t *implicitType)
{
    const h2_t *type = implicitType ? implicitType : get_int_type(eDigitInt, eSignSigned);
    return h2_expr_digit(expr->node, type, expr->intValue);
}

static h2_t *ctu_sema_noinit(h2_t *sema, ctu_t *expr, const h2_t *implicitType)
{
    if (implicitType == NULL)
    {
        report(sema->reports, eFatal, expr->node, "no implicit type in this context for noinit");
        return h2_error(expr->node, "no implicit type");
    }

    return h2_expr_empty(expr->node, implicitType);
}

static h2_t *ctu_sema_lvalue(h2_t *sema, ctu_t *expr)
{

}

static h2_t *ctu_sema_rvalue(h2_t *sema, ctu_t *expr, const h2_t *implicitType)
{
    switch (expr->kind)
    {
    case eCtuExprInt: return ctu_sema_int(sema, expr, implicitType);
    case eCtuExprNoInit: return ctu_sema_noinit(sema, expr, implicitType);

    default: NEVER("invalid rvalue-expr kind %d", expr->kind);
    }
}

///
/// resolving
///

static void ctu_resolve_global(h2_cookie_t *cookie, h2_t *self, void *user)
{

}

static void ctu_resolve_function(h2_cookie_t *cookie, h2_t *self, void *user)
{

}

///
/// forwarding
///

static h2_t *ctu_forward_global(h2_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclGlobal, "decl %s is not a global", decl->name);

    h2_t *type = ctu_sema_type(sema, decl->type);
    h2_t *global = h2_open_global(decl->node, decl->name, type, decl, ctu_resolve_global);

    return global;
}

static h2_t *ctu_forward_function(h2_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclFunction, "decl %s is not a function", decl->name);

    h2_t *returnType = ctu_sema_type(sema, decl->returnType);
    h2_t *signature = h2_type_closure(decl->node, decl->name, returnType, vector_of(0), eArityFixed);

    h2_t *function = h2_open_function(decl->node, decl->name, signature, decl, ctu_resolve_function);

    return function;
}

typedef struct ctu_forward_t {
    ctu_tag_t tag;
    h2_t *decl;
} ctu_forward_t;

static ctu_forward_t ctu_forward_decl(h2_t *sema, ctu_t *decl)
{
    switch (decl->kind)
    {
    case eCtuDeclGlobal: {
        ctu_forward_t fwd = {
            .tag = eTagValues,
            .decl = ctu_forward_global(sema, decl),
        };
        return fwd;
    }
    case eCtuDeclFunction: {
        ctu_forward_t fwd = {
            .tag = eTagFunctions,
            .decl = ctu_forward_function(sema, decl),
        };
        return fwd;
    }

    default:
        NEVER("invalid decl kind %d", decl->kind);
    }
}

void ctu_forward_decls(context_t *context)
{
    lifetime_t *lifetime = context_get_lifetime(context);
    ctu_t *ast = context_get_ast(context);
    const char *name = context_get_name(context);
    reports_t *reports = lifetime_get_reports(lifetime);

    vector_t *decls = ast->decls;
    size_t len = vector_len(decls);

    size_t sizes[eTagTotal] = {
        [eTagValues] = len,
        [eTagTypes] = len,
        [eTagFunctions] = len,
        [eTagModules] = len,
        [eTagImports] = vector_len(ast->imports),
        [eTagAttribs] = len,
        [eTagSuffix] = len,
    };

    h2_t *mod = h2_module(kRootModule, ast->node, name, eTagTotal, sizes);

    for (size_t i = 0; i < len; i++)
    {
        ctu_t *decl = vector_get(decls, i);

        if (is_discard(decl->name))
        {
            report(reports, eFatal, decl->node, "top-level declarations must have a name");
        }
        else
        {
            ctu_forward_t fwd = ctu_forward_decl(mod, decl);
            add_decl(mod, fwd.tag, decl->name, fwd.decl);
        }
    }

    context_update(context, ast, mod);
}

static char *capitalize(const char *name)
{
    CTASSERT(strlen(name) > 0);

    char *copy = ctu_strdup(name);
    copy[0] = toupper(copy[0]);
    return copy;
}

static void import_module(lifetime_t *lifetime, h2_t *sema, ctu_t *include)
{
    CTASSERT(include->kind == eCtuImport);
    context_t *ctx = get_context(lifetime, include->importPath);

    if (ctx == NULL)
    {
        report(sema->reports, eFatal, include->node, "import %s not found", str_join("::", include->importPath));
        return;
    }

    h2_t *lib = context_get_module(ctx);
    if (lib == sema)
    {
        report(sema->reports, eFatal, include->node, "module cannot import itself");
        return;
    }

    h2_t *old = get_namespace(sema, include->name);
    if (old != NULL)
    {
        message_t *id = report_shadow(sema->reports, include->name, h2_get_node(old), h2_get_node(lib));
        report_note(id, "consider using import aliases; eg. `import %s as inc%s`",
            str_join("::", include->importPath),
            capitalize(include->name)
        );
    }
    else
    {
        add_decl(sema, eTagImports, include->name, lib);
    }
}

void ctu_process_imports(context_t *context)
{
    lifetime_t *lifetime = context_get_lifetime(context);

    ctu_t *ast = context_get_ast(context);
    h2_t *sema = context_get_module(context);

    size_t len = vector_len(ast->imports);
    for (size_t i = 0; i < len; i++)
    {
        ctu_t *it = vector_get(ast->imports, i);
        import_module(lifetime, sema, it);
    }
}

void ctu_compile_module(context_t *context)
{
    h2_t *sema = context_get_module(context);

    map_iter_t globals = map_iter(h2_module_tag(sema, eTagValues));
    while (map_has_next(&globals))
    {
        map_entry_t entry = map_next(&globals);
        h2_t *global = entry.value;
        CTASSERTF(h2_is(global, eHlir2Resolve), "expected resolve, got %s", h2_to_string(global));

        ctu_t *decl = global->user;
        const h2_t *type = h2_get_type(global);
        h2_t *value = ctu_sema_rvalue(sema, decl->global, type);

        h2_close_global(global, value);
    }

    map_iter_t functions = map_iter(h2_module_tag(sema, eTagFunctions));
    while (map_has_next(&functions))
    {
        map_entry_t entry = map_next(&functions);
        h2_t *function = entry.value;

        h2_t *body = h2_stmt_block(function->node, vector_of(0));
        h2_close_function(function, body);
    }
}
