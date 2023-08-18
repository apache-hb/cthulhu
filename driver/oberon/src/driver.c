#include "oberon/driver.h"
#include "oberon/ast.h"

#include "oberon/sema/type.h"
#include "oberon/sema/decl.h"

#include "cthulhu/mediator/driver.h"

#include "cthulhu/tree/query.h"

#include "report/report.h"
#include "report/report-ext.h"

#include "std/map.h"
#include "std/str.h"

#include "base/panic.h"

static tree_t *gRuntime = NULL;

void obr_create(driver_t *handle)
{
    lifetime_t *lifetime = handle_get_lifetime(handle);
    gRuntime = obr_rt_mod(lifetime);
}

void obr_forward_decls(context_t *context)
{
    obr_t *root = context_get_ast(context);
    size_t lenDecls = vector_len(root->decls);

    size_t sizes[eObrTagTotal] = {
        [eObrTagValues] = lenDecls,
        [eObrTagTypes] = lenDecls,
        [eObrTagProcs] = lenDecls,
        [eObrTagModules] = 32,
    };

    tree_t *sema = tree_module(gRuntime, root->node, root->name, eObrTagTotal, sizes);

    for (size_t i = 0; i < lenDecls; i++)
    {
        obr_t *decl = vector_get(root->decls, i);
        obr_forward_t fwd = obr_forward_decl(sema, decl);
        tree_t *it = fwd.decl;

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
