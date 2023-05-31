#include "cthulhu/mediator/language.h"
#include "scan/compile.h"

#include "base/macros.h"

#include "cc-bison.h"
#include "cc-flex.h"

CT_CALLBACKS(kCallbacks, cc);

static void *cc_parse(lang_handle_t *lang, scan_t *scan)
{
    UNUSED(lang);

    cc_init_scan(scan);
    return compile_scanner(scan, &kCallbacks);
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

    .fnParse = cc_parse
};
