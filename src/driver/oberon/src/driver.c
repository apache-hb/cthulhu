#include "oberon/driver.h"
#include "cthulhu/events/events.h"
#include "oberon/ast.h"

#include "oberon/sema/decl.h"

#include "cthulhu/runtime/driver.h"

#include "cthulhu/tree/query.h"

#include "base/panic.h"

#include "core/macros.h"

static tree_t *gRuntime = NULL;

void obr_create(driver_t *handle)
{
    lifetime_t *lifetime = handle_get_lifetime(handle);
    gRuntime = obr_rt_mod(lifetime);
}

void obr_forward_decls(context_t *context)
{
    obr_t *root = context_get_ast(context);
    size_t decl_count = vector_len(root->decls);

    size_t sizes[eObrTagTotal] = {
        [eObrTagValues] = decl_count,
        [eObrTagTypes] = decl_count,
        [eObrTagProcs] = decl_count,
        [eObrTagModules] = 32,
    };

    tree_t *sema = tree_module(gRuntime, root->node, root->name, eObrTagTotal, sizes);

    for (size_t i = 0; i < decl_count; i++)
    {
        obr_t *decl = vector_get(root->decls, i);
        obr_forward_t fwd = obr_forward_decl(sema, decl);
        tree_t *it = fwd.decl;

        obr_add_decl(sema, fwd.tag, it->name, it);
    }

    tree_t *init = obr_add_init(sema, root);
    if (init != NULL)
    {
        obr_add_decl(sema, eObrTagProcs, init->name, init); // TODO: pick a better name
    }

    context_update(context, root, sema);
}

static void import_module(lifetime_t *lifetime, tree_t *sema, obr_t *include)
{
    CTASSERT(include->kind == eObrImport);
    arena_t *arena = lifetime_get_arena(lifetime);
    context_t *ctx = get_context(lifetime, vector_init_arena(include->symbol, arena));

    if (ctx == NULL)
    {
        msg_notify(sema->reports, &kEvent_ImportNotFound, include->node, "failed to find context for module '%s'", include->symbol);
        return;
    }

    tree_t *lib = context_get_module(ctx);
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

void obr_process_imports(context_t *context)
{
    lifetime_t *lifetime = context_get_lifetime(context);
    obr_t *root = context_get_ast(context);
    tree_t *sema = context_get_module(context);

    size_t len = vector_len(root->imports);
    for (size_t i = 0; i < len; i++)
    {
        obr_t *import = vector_get(root->imports, i);
        import_module(lifetime, sema, import);
    }
}

void obr_compile_module(context_t *context)
{
    CTU_UNUSED(context);
}
