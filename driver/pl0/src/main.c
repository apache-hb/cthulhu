#include "cthulhu/mediator/driver.h"

#include "scan/compile.h"

#include "sema.h"
#include "ast.h"

#include "base/macros.h"

#include "pl0-bison.h"
#include "pl0-flex.h"

CT_CALLBACKS(kCallbacks, pl0);

static void *pl0_parse(lifetime_t *lifetime, scan_t *scan)
{
    pl0_t *ast = compile_scanner(scan, &kCallbacks);
    CTASSERT(ast->type == ePl0Module);

    vector_t *path = vector_init((char*)ast->mod);

    context_t *ctx = context_new(lifetime, ast, NULL);

    add_context(lifetime, path, ctx);
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

    .fnCreate = pl0_init,

    .fnParse = pl0_parse,
    .fnForward = pl0_forward_decls,
    .fnImport = pl0_process_imports,
    .fnCompile = pl0_compile_module
};
