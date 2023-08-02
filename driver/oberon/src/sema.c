#include "oberon/sema.h"
#include "oberon/ast.h"

#include "cthulhu/mediator/driver.h"

#include "cthulhu/hlir/query.h"

#include "report/report.h"
#include "report/report-ext.h"

#include "std/str.h"

#include "base/panic.h"

static const h2_attrib_t kDefaultGlobalAttribs = {
    .link = eLinkExport,
    .visibility = eVisiblePublic
};

static h2_t *obr_forward_global(h2_t *sema, obr_t *global)
{
    CTASSERTF(global->kind == eObrDeclGlobal, "decl %s is not a global", global->name);

    mpz_t zero;
    mpz_init(zero);

    h2_t *intType = h2_type_digit(global->node, "INTEGER", eDigitInt, eSignSigned);
    h2_t *mut = h2_qualify(global->node, intType, eQualMutable);
    h2_t *zeroLiteral = h2_expr_digit(global->node, intType, zero);
    h2_t *decl = h2_decl_global(global->node, global->name, mut, zeroLiteral);
    h2_set_attrib(decl, &kDefaultGlobalAttribs);

    return decl;
}

static void add_decl(h2_t *sema, obr_tags_t tag, const char *name, h2_t *decl)
{
    const h2_t *old = h2_module_get(sema, tag, name);
    if (old != NULL)
    {
        report_shadow(sema->reports, name, h2_get_node(old), h2_get_node(decl));
    }
    else
    {
        h2_module_set(sema, tag, name, decl);
    }
}

void obr_forward_decls(context_t *context)
{
    lifetime_t *lifetime = context_get_lifetime(context);
    reports_t *reports = lifetime_get_reports(lifetime);

    obr_t *root = context_get_ast(context);
    size_t lenDecls = vector_len(root->decls);

    size_t sizes[eTagTotal] = {
        [eTagValues] = lenDecls,
        [eTagTypes] = lenDecls,
        [eTagProcs] = lenDecls,
        [eTagModules] = 32,
    };

    h2_t *sema = h2_module_root(reports, root->node, root->name, eTagTotal, sizes);

    for (size_t i = 0; i < lenDecls; i++)
    {
        obr_t *global = vector_get(root->decls, i);
        h2_t *decl = obr_forward_global(sema, global);
        add_decl(sema, eTagValues, global->name, decl);
    }

    context_update(context, root, sema);
}

void obr_process_imports(context_t *context)
{

}

void obr_compile_module(context_t *context)
{

}
