#include "pl0/sema.h"
#include "pl0/ast.h"

#include "scan/compile.h"

#include "std/str.h"

#include "base/macros.h"

#include "pl0-bison.h"
#include "pl0-flex.h"

CT_CALLBACKS(kCallbacks, pl0);

static void pl0_parse(driver_t *handle, scan_t *scan)
{
    lifetime_t *lifetime = handle_get_lifetime(handle);
    pl0_t *ast = compile_scanner(scan, &kCallbacks);
    if (ast == NULL) { return; }

    CTASSERT(ast->type == ePl0Module);

    // TODO: dedup this with pl0_forward_decls
    char *fp = (char*)scan_path(scan);
    vector_t *path = vector_len(ast->mod) > 0
        ? ast->mod
        : vector_init(str_filename_noext(fp));

    context_t *ctx = context_new(handle, vector_tail(path), ast, NULL);

    add_context(lifetime, path, ctx);
}

static void pl0_config(lifetime_t *lifetime, ap_t *ap)
{
    CTU_UNUSED(lifetime);
    CTU_UNUSED(ap);
}

static const char *kLangNames[] = { "pl", "pl0", NULL };

const language_t kPl0Module = {
    .id = "pl0",
    .name = "PL/0",
    .version = {
        .license = "LGPLv3",
        .desc = "PL/0 language driver",
        .author = "Elliot Haisley",
        .version = NEW_VERSION(2, 3, 2)
    },

    .exts = kLangNames,

    .fnConfig = pl0_config,
    .fnCreate = pl0_init,

    .fnParse = pl0_parse,
    .fnCompilePass = {
        [eStageForwardSymbols] = pl0_forward_decls,
        [eStageCompileImports] = pl0_process_imports,
        [eStageCompileSymbols] = pl0_compile_module
    }
};
