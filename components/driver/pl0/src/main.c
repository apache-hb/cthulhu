#include "cthulhu/mediator/language.h"

#include "scan/compile.h"

#include "sema.h"

#include "base/macros.h"

#include "pl0-bison.h"
#include "pl0-flex.h"

CT_CALLBACKS(kCallbacks, pl0);

static void *pl0_parse(lang_handle_t *handle, context_t *ctx)
{
    return compile_scanner(context_get_scanner(ctx), &kCallbacks);
}

static const char *kLangNames[] = { "pl", "pl0", NULL };

static const language_t kLanguageInfo = {
    .id = "pl0",
    .name = "PL/0",
    .version = {
        .license = "LGPLv3",
        .desc = "PL/0 language driver",
        .author = "Elliot Haisley",
        .version = NEW_VERSION(2, 3, 1)
    },

    .exts = kLangNames,

    .fnInit = pl0_init,

    .fnParse = pl0_parse,
    .fnForward = pl0_forward_decls,
    .fnImport = pl0_process_imports,
    .fnCompile = pl0_compile_module
};

LANGUAGE_EXPORT
extern const language_t *LANGUAGE_ENTRY_POINT(mediator_t *lang)
{
    UNUSED(lang);

    return &kLanguageInfo;
}
