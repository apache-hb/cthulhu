#include "cthulhu/mediator/language.h"
#include "scan/compile.h"

#include "base/macros.h"

#include "report/report.h"

#include "cc-bison.h"
#include "cc-flex.h"

CT_CALLBACKS(kCallbacks, cc);

static void *cc_parse(lang_handle_t *lang, scan_t *scan)
{
    CTU_UNUSED(lang);

    cc_init_scan(scan);
    return compile_scanner(scan, &kCallbacks);
}

static void cc_forward(lang_handle_t *handle, const char *name, void *ast)
{
    report(lang_get_reports(handle), eSorry, NULL, "C is unimplemented");
}

static void cc_imports(lang_handle_t *handle, compile_t *compile)
{
    report(lang_get_reports(handle), eSorry, NULL, "C is unimplemented");
}

static tree_t *cc_compile(lang_handle_t *handle, compile_t *compile)
{
    report(lang_get_reports(handle), eSorry, NULL, "C is unimplemented");
    return NULL;
}

static const char *kLangNames[] = { "c", "h", NULL };

const language_t kCModule = {
    .id = "c",
    .name = "C",
    .version = {
        .license = "LGPLv3",
        .desc = "C11 language driver",
        .author = "Elliot Haisley",
        .version = NEW_VERSION(0, 0, 1),
    },

    .exts = kLangNames,

    .fnParse = cc_parse,

    .fnForward = cc_forward,
    .fnImport = cc_imports,
    .fnCompile = cc_compile,
};
