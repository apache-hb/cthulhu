#include "ctu/driver.h"
#include "cthulhu/broker/broker.h"
#include "cthulhu/events/events.h"
#include "ctu/ast.h"

#include "ctu/sema/sema.h"
#include "ctu/sema/decl.h"

#include "cthulhu/tree/tree.h"
#include "cthulhu/tree/query.h"

#include "std/vector.h"
#include "std/map.h"
#include "std/str.h"

#include "base/panic.h"

#include "core/macros.h"

#include <string.h>

///
/// init
///

static tree_t *gRootModule = NULL;

void ctu_init(language_runtime_t *runtime, tree_t *root)
{
    ctu_rt_mod(runtime, root);
}

void ctu_forward_decls(language_runtime_t *runtime, compile_unit_t *unit)
{
    ctu_t *ast = unit_get_ast(unit);
    const char *name = unit_get_name(unit);

    const vector_t *decls = ast->decls;
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

    unit_update(unit, ast, mod);
}

static void import_module(language_runtime_t *lifetime, tree_t *sema, ctu_t *include)
{
    CTASSERT(include->kind == eCtuImport);
    arena_t *arena = lang_get_arena(lifetime);
    compile_unit_t *ctx = lang_get_unit(lifetime, include->import_path);

    if (ctx == NULL)
    {
        msg_notify(sema->reports, &kEvent_ImportNotFound, include->node, "import `%s` not found", str_join("::", include->import_path, arena));
        return;
    }

    tree_t *lib = unit_get_tree(ctx);
    if (lib == sema)
    {
        msg_notify(sema->reports, &kEvent_CirclularImport, include->node, "module cannot import itself");
        return;
    }

    tree_t *old = ctu_get_namespace(sema, include->name, NULL);
    if (old != NULL)
    {
        event_builder_t id = evt_symbol_shadowed(sema->reports, include->name, tree_get_node(old), tree_get_node(lib));
        msg_note(id, "consider using import aliases; eg. `import %s as my_%s`",
            str_join("::", include->import_path, arena),
            include->name
        );
    }
    else
    {
        ctu_add_decl(sema, eCtuTagImports, include->name, lib);
    }
}

void ctu_process_imports(language_runtime_t *runtime, compile_unit_t *unit)
{
    ctu_t *ast = unit_get_ast(unit);
    tree_t *sema = unit_get_tree(unit);

    size_t len = vector_len(ast->imports);
    for (size_t i = 0; i < len; i++)
    {
        ctu_t *it = vector_get(ast->imports, i);
        import_module(runtime, sema, it);
    }
}

void ctu_compile_module(language_runtime_t *runtime, compile_unit_t *unit)
{
    CT_UNUSED(runtime);
    CT_UNUSED(unit);
}
