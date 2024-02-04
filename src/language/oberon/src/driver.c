#include "oberon/driver.h"
#include "cthulhu/broker/broker.h"
#include "cthulhu/events/events.h"
#include "oberon/ast.h"

#include "oberon/sema/decl.h"

#include "cthulhu/tree/query.h"

#include "base/panic.h"

#include "core/macros.h"

void obr_forward_decls(language_runtime_t *runtime, compile_unit_t *unit)
{
    tree_t *root = lang_get_root(runtime);
    obr_t *ast = unit_get_ast(unit);
    size_t decl_count = vector_len(ast->decls);

    size_t sizes[eObrTagTotal] = {
        [eObrTagValues] = decl_count,
        [eObrTagTypes] = decl_count,
        [eObrTagProcs] = decl_count,
        [eObrTagModules] = 32,
    };

    tree_t *sema = tree_module(root, root->node, root->name, eObrTagTotal, sizes);

    for (size_t i = 0; i < decl_count; i++)
    {
        obr_t *decl = vector_get(ast->decls, i);
        obr_forward_t fwd = obr_forward_decl(sema, decl);
        tree_t *it = fwd.decl;

        obr_add_decl(sema, fwd.tag, it->name, it);
    }

    tree_t *init = obr_add_init(sema, ast);
    if (init != NULL)
    {
        obr_add_decl(sema, eObrTagProcs, init->name, init); // TODO: pick a better name
    }

    unit_update(unit, ast, sema);
}

static void import_module(language_runtime_t *runtime, tree_t *sema, obr_t *include)
{
    CTASSERT(include->kind == eObrImport);
    arena_t *arena = lang_get_arena(runtime);
    compile_unit_t *ctx = lang_get_unit(runtime, vector_init(include->symbol, arena));

    if (ctx == NULL)
    {
        msg_notify(sema->reports, &kEvent_ImportNotFound, include->node, "failed to find context for module '%s'", include->symbol);
        return;
    }

    tree_t *lib = unit_get_tree(ctx);
    if (lib == sema)
    {
        msg_notify(sema->reports, &kEvent_CirclularImport, include->node, "module cannot import itself");
        return;
    }

    tree_t *old = obr_get_namespace(sema, include->name);
    if (old != NULL)
    {
        event_builder_t id = evt_symbol_shadowed(sema->reports, include->name, tree_get_node(old), tree_get_node(lib));
        msg_note(id, "consider using import aliases; eg. `IMPORT my_%s := %s;", include->name, include->symbol);
    }
    else
    {
        obr_add_decl(sema, eObrTagImports, include->name, lib);
    }
}

void obr_process_imports(language_runtime_t *runtime, compile_unit_t *unit)
{
    obr_t *root = unit_get_ast(unit);
    tree_t *sema = unit_get_tree(unit);

    size_t len = vector_len(root->imports);
    for (size_t i = 0; i < len; i++)
    {
        obr_t *import = vector_get(root->imports, i);
        import_module(runtime, sema, import);
    }
}

void obr_compile_module(language_runtime_t *runtime, compile_unit_t *unit)
{
    CT_UNUSED(runtime);
    CT_UNUSED(unit);
}
