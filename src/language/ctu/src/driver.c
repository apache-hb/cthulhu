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

void ctu_init(language_runtime_t *runtime, tree_t *root)
{
    ctu_rt_mod(runtime, root);
}

void ctu_forward_decls(language_runtime_t *runtime, compile_unit_t *unit)
{
    CT_UNUSED(runtime);

    ctu_t *ast = unit_get_ast(unit);

    const vector_t *decls = ast->decls;
    size_t len = vector_len(decls);
    tree_t *mod = unit->tree;

    for (size_t i = 0; i < len; i++)
    {
        ctu_t *decl = vector_get(decls, i);

        ctu_forward_t fwd = ctu_forward_decl(mod, decl);
        ctu_add_decl(mod, fwd.tag, decl->name, fwd.decl);
    }

    unit_update(unit, ast, mod);
}

static tree_t *get_import(tree_t *sema, const char *name)
{
    map_t *imports = tree_module_tag(sema, eCtuTagImports);
    tree_t *it = map_get(imports, name);
    if (it != NULL)
        return it;

    map_t *mods = tree_module_tag(sema, eCtuTagModules);
    return map_get(mods, name);
}

static void import_module(language_runtime_t *runtime, tree_t *sema, ctu_t *include)
{
    CTASSERT(include->kind == eCtuImport);
    arena_t *arena = runtime->arena;
    unit_id_t id = build_unit_id(include->import_path, arena);
    compile_unit_t *ctx = lang_get_unit(runtime, id);

    if (ctx == NULL)
    {
        msg_notify(sema->reports, &kEvent_ImportNotFound, include->node, "import `%s` not found", str_join("::", include->import_path, arena));
        return;
    }

    if (ctx->tree == sema)
    {
        msg_notify(sema->reports, &kEvent_CirclularImport, include->node, "module cannot import itself");
        return;
    }

    // only search the current module for the import
    // if we did this recursively we would reach the root module
    // which always has the imported module
    tree_t *old = get_import(sema, include->name);
    if (old != NULL)
    {
        event_builder_t evt = evt_symbol_shadowed(sema->reports, include->name, tree_get_node(old), include->node);
        msg_note(evt, "consider using import aliases; eg. `import %s as my_%s`",
            str_join("::", include->import_path, arena),
            include->name
        );
    }
    else
    {
        ctu_add_decl(sema, eCtuTagImports, include->name, ctx->tree);
    }
}

void ctu_process_imports(language_runtime_t *runtime, compile_unit_t *unit)
{
    ctu_t *ast = unit_get_ast(unit);

    size_t len = vector_len(ast->imports);
    for (size_t i = 0; i < len; i++)
    {
        ctu_t *it = vector_get(ast->imports, i);
        import_module(runtime, unit->tree, it);
    }
}

void ctu_compile_module(language_runtime_t *runtime, compile_unit_t *unit)
{
    CT_UNUSED(runtime);
    CT_UNUSED(unit);
}
