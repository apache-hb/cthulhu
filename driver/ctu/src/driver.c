#include "ctu/driver.h"
#include "ctu/ast.h"

#include "ctu/sema/sema.h"
#include "ctu/sema/type.h"
#include "ctu/sema/decl.h"
#include "ctu/sema/expr.h"

#include "base/util.h"

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
/// init
///

static h2_t *kRootModule = NULL;

void ctu_init(driver_t *handle)
{
    lifetime_t *lifetime = handle_get_lifetime(handle);

    kRootModule = ctu_rt_mod(lifetime);
    vector_t *path = ctu_rt_path();

    context_t *ctx = compiled_new(handle, kRootModule);
    add_context(lifetime, path, ctx);
}

void ctu_forward_decls(context_t *context)
{
    ctu_t *ast = context_get_ast(context);
    const char *name = context_get_name(context);

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

        ctu_forward_t fwd = ctu_forward_decl(mod, decl);
        ctu_add_decl(mod, fwd.tag, decl->name, fwd.decl);
    }

    context_update(context, ast, mod);
}

static void import_module(lifetime_t *lifetime, h2_t *sema, ctu_t *include)
{
    CTASSERT(include->kind == eCtuImport);
    context_t *ctx = get_context(lifetime, include->importPath);

    if (ctx == NULL)
    {
        report(sema->reports, eFatal, include->node, "import `%s` not found", str_join("::", include->importPath));
        return;
    }

    h2_t *lib = context_get_module(ctx);
    if (lib == sema)
    {
        report(sema->reports, eFatal, include->node, "module cannot import itself");
        return;
    }

    h2_t *old = ctu_get_namespace(sema, include->name);
    if (old != NULL)
    {
        message_t *id = report_shadow(sema->reports, include->name, h2_get_node(old), h2_get_node(lib));
        report_note(id, "consider using import aliases; eg. `import %s as my_%s`",
            str_join("::", include->importPath),
            include->name
        );
    }
    else
    {
        ctu_add_decl(sema, eTagImports, include->name, lib);
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
    h2_cookie_t *cookie = h2_get_cookie(sema);

    map_iter_t globals = map_iter(h2_module_tag(sema, eTagValues));
    while (map_has_next(&globals))
    {
        map_entry_t entry = map_next(&globals);
        h2_resolve(cookie, entry.value);
    }

    map_iter_t functions = map_iter(h2_module_tag(sema, eTagFunctions));
    while (map_has_next(&functions))
    {
        map_entry_t entry = map_next(&functions);
        h2_resolve(cookie, entry.value);
    }
}
