#include "ctu/driver.h"
#include "ctu/ast.h"

#include "ctu/sema/sema.h"
#include "ctu/sema/decl.h"

#include "cthulhu/mediator/driver.h"

#include "cthulhu/tree/tree.h"
#include "cthulhu/tree/query.h"

#include "std/vector.h"
#include "std/map.h"
#include "std/str.h"

#include "base/panic.h"

#include "core/macros.h"

#include <ctype.h>
#include <string.h>

///
/// init
///

static tree_t *gRootModule = NULL;

void ctu_init(driver_t *handle)
{
    lifetime_t *lifetime = handle_get_lifetime(handle);

    gRootModule = ctu_rt_mod(lifetime);
    vector_t *path = ctu_rt_path();

    context_t *ctx = compiled_new(handle, gRootModule);
    add_context(lifetime, path, ctx);
}

void ctu_forward_decls(context_t *context)
{
    ctu_t *ast = context_get_ast(context);
    const char *name = context_get_name(context);

    vector_t *decls = ast->decls;
    size_t len = vector_len(decls);

    size_t sizes[eCtuTagTotal] = {
        [eCtuTagValues] = len,
        [eCtuTagTypes] = len,
        [eCtuTagFunctions] = len,
        [eCtuTagModules] = len,
        [eCtuTagImports] = vector_len(ast->imports),
        [eCtuTagAttribs] = len,
        [eCtuTagSuffixes] = len,
    };

    tree_t *mod = tree_module(gRootModule, ast->node, name, eCtuTagTotal, sizes);

    for (size_t i = 0; i < len; i++)
    {
        ctu_t *decl = vector_get(decls, i);

        ctu_forward_t fwd = ctu_forward_decl(mod, decl);
        ctu_add_decl(mod, fwd.tag, decl->name, fwd.decl);
    }

    context_update(context, ast, mod);
}

static void import_module(lifetime_t *lifetime, tree_t *sema, ctu_t *include)
{
    CTASSERT(include->kind == eCtuImport);
    context_t *ctx = get_context(lifetime, include->importPath);

    if (ctx == NULL)
    {
        report(sema->reports, eFatal, include->node, "import `%s` not found", str_join("::", include->importPath));
        return;
    }

    tree_t *lib = context_get_module(ctx);
    if (lib == sema)
    {
        report(sema->reports, eFatal, include->node, "module cannot import itself");
        return;
    }

    tree_t *old = ctu_get_namespace(sema, include->name, NULL);
    if (old != NULL)
    {
        message_t *id = report_shadow(sema->reports, include->name, tree_get_node(old), tree_get_node(lib));
        report_note(id, "consider using import aliases; eg. `import %s as my_%s`",
            str_join("::", include->importPath),
            include->name
        );
    }
    else
    {
        ctu_add_decl(sema, eCtuTagImports, include->name, lib);
    }
}

void ctu_process_imports(context_t *context)
{
    lifetime_t *lifetime = context_get_lifetime(context);

    ctu_t *ast = context_get_ast(context);
    tree_t *sema = context_get_module(context);

    size_t len = vector_len(ast->imports);
    for (size_t i = 0; i < len; i++)
    {
        ctu_t *it = vector_get(ast->imports, i);
        import_module(lifetime, sema, it);
    }
}

void ctu_compile_module(context_t *context)
{
    CTU_UNUSED(context);
}
