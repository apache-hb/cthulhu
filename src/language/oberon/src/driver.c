#include "oberon/driver.h"
#include "base/util.h"
#include "cthulhu/broker/broker.h"
#include "cthulhu/events/events.h"
#include "oberon/ast.h"

#include "oberon/sema/decl.h"

#include "cthulhu/tree/query.h"

#include "base/panic.h"

#include "core/macros.h"

void obr_forward_decls(language_runtime_t *runtime, compile_unit_t *unit)
{
    CT_UNUSED(runtime);

    obr_t *ast = unit_get_ast(unit);
    size_t decl_count = vector_len(ast->decls);

    tree_t *sema = unit->tree;

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
        // TODO: pick a better name
        obr_add_decl(sema, eObrTagProcs, init->name, init);
    }

    unit_update(unit, ast, sema);
}

static void import_module(language_runtime_t *runtime, tree_t *sema, obr_t *include)
{
    CTASSERT(include->kind == eObrImport);
    unit_id_t id = text_view_from(include->symbol);
    compile_unit_t *ctx = lang_get_unit(runtime, id);

    if (ctx == NULL)
    {
        msg_notify(sema->reports, &kEvent_ImportNotFound, include->node, "failed to find context for module '%s'", include->symbol);
        return;
    }

    if (ctx->tree == sema)
    {
        msg_notify(sema->reports, &kEvent_CirclularImport, include->node, "module cannot import itself");
        return;
    }

    // only search current module for the import

    tree_t *old = obr_get_namespace(sema, include->name);
    if (old != NULL)
    {
        event_builder_t evt = evt_symbol_shadowed(sema->reports, include->name, tree_get_node(old), include->node);
        msg_note(evt, "consider using import aliases; eg. `IMPORT my_%s := %s;", include->name, include->symbol);
    }
    else
    {
        obr_add_decl(sema, eObrTagImports, include->name, ctx->tree);
    }
}

void obr_process_imports(language_runtime_t *runtime, compile_unit_t *unit)
{
    obr_t *root = unit_get_ast(unit);

    size_t len = vector_len(root->imports);
    for (size_t i = 0; i < len; i++)
    {
        obr_t *import = vector_get(root->imports, i);
        import_module(runtime, unit->tree, import);
    }
}
