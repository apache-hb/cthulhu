#include "oberon/sema.h"
#include "oberon/ast.h"

#include "oberon/sema/type.h"
#include "oberon/sema/decl.h"

#include "cthulhu/mediator/driver.h"

#include "cthulhu/hlir/query.h"

#include "report/report.h"
#include "report/report-ext.h"

#include "std/map.h"
#include "std/str.h"

#include "base/panic.h"

static h2_t *gRuntime = NULL;

static h2_t *gTypeInteger = NULL;

void obr_create(driver_t *handle)
{
    lifetime_t *lifetime = handle_get_lifetime(handle);

    size_t tags[eTagTotal] = {
        [eTagValues] = 32,
        [eTagTypes] = 32,
        [eTagProcs] = 32,
        [eTagModules] = 32,
    };

    gTypeInteger = h2_type_digit(node_builtin(), "INTEGER", eDigitInt, eSignSigned);

    gRuntime = lifetime_sema_new(lifetime, "oberon", eTagTotal, tags);
    obr_add_decl(gRuntime, eTagTypes, "INTEGER", gTypeInteger);
}

void obr_forward_decls(context_t *context)
{
    obr_t *root = context_get_ast(context);
    size_t lenDecls = vector_len(root->decls);

    size_t sizes[eTagTotal] = {
        [eTagValues] = lenDecls,
        [eTagTypes] = lenDecls,
        [eTagProcs] = lenDecls,
        [eTagModules] = 32,
    };

    h2_t *sema = h2_module(gRuntime, root->node, root->name, eTagTotal, sizes);

    for (size_t i = 0; i < lenDecls; i++)
    {
        obr_t *decl = vector_get(root->decls, i);
        obr_forward_t fwd = obr_forward_decl(sema, decl);
        h2_t *it = fwd.decl;

        obr_add_decl(sema, fwd.tag, it->name, it);
    }

    context_update(context, root, sema);
}

void obr_process_imports(context_t *context)
{

}

void obr_compile_module(context_t *context)
{

}
