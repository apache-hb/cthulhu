#include "ctu/sema.h"
#include "ctu/ast.h"

#include "cthulhu/mediator/driver.h"

#include "cthulhu/hlir/h2.h"
#include "cthulhu/hlir/query.h"

#include "report/report.h"
#include "report/report-ext.h"

#include "std/vector.h"

#include "base/panic.h"

static bool is_discard(const char *name)
{
    return name == NULL;
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

static h2_t *ctu_forward_global(h2_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclGlobal, "decl %s is not a global", decl->name);

    h2_t *type = h2_type_digit(decl->node, "int", eDigitInt, eSignSigned);
    h2_t *global = h2_decl_global(decl->node, decl->name, type, NULL);

    return global;
}

static h2_t *ctu_forward_function(h2_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuDeclFunction, "decl %s is not a function", decl->name);

    h2_t *returnType = h2_type_digit(decl->node, "int", eDigitInt, eSignSigned);
    h2_t *signature = h2_type_closure(decl->node, decl->name, returnType, vector_of(0), eArityFixed);

    h2_t *function = h2_decl_function(decl->node, decl->name, signature, NULL);

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
        [eTagAttribs] = len,
        [eTagSuffix] = len,
    };

    h2_t *mod = h2_module_root(reports, ast->node, name, eTagTotal, sizes);

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

void ctu_process_imports(context_t *context)
{
    CTU_UNUSED(context);
}

void ctu_compile_module(context_t *context)
{
    CTU_UNUSED(context);
}
