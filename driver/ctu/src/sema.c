#include "ctu/sema.h"
#include "ctu/ast.h"

#include "std/vector.h"

#include "base/panic.h"

#include "report/report.h"

#include "cthulhu/mediator/driver.h"

#include "cthulhu/hlir/h2.h"
#include "cthulhu/hlir/query.h"

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
        message_t *id = report(sema->reports, eFatal, decl->node, "redefinition of '%s'", name);
        report_append(id, h2_get_node(old), "previous definition");
    }
    else
    {
        h2_module_set(sema, tag, name, decl);
    }
}

static h2_t *ctu_forward_global(h2_t *sema, ctu_t *decl)
{
    CTASSERTF(decl->kind == eCtuGlobal, "decl %s is not a global", decl->name);

    h2_t *type = h2_type_digit(decl->node, "int", eDigitInt, eSignSigned);
    h2_t *global = h2_decl_global(decl->node, decl->name, type, NULL);

    return global;
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
            h2_t *fwd = ctu_forward_global(mod, decl);
            add_decl(mod, eTagValues, decl->name, fwd);
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
