#include "cthulhu/mediator/language.h"
#include "scan/compile.h"

#include "sema/sema.h"
#include "sema/config.h"

#include "base/macros.h"

#include "ctu-bison.h"
#include "ctu-flex.h"

CT_CALLBACKS(kCallbacks, ctu);

static void *ctu_parse_file(lang_handle_t *runtime, scan_t *scan)
{
    UNUSED(runtime);

    ctu_init_scan(scan);
    return compile_scanner(scan, &kCallbacks);
}

static void ctu_configure(lang_handle_t *handle, ap_t *ap)
{
    ctu_config_init(handle, ap);
}

static const char *kLangNames[] = { "ct", "ctu", NULL };

const language_t kCtuModule = {
    .id = "ctu",
    .name = "Cthulhu",
    .version = {
        .license = "LGPLv3",
        .desc = "Cthulhu language driver",
        .author = "Elliot Haisley",
        .version = NEW_VERSION(0, 4, 0)
    },

    .exts = kLangNames,

    .fnConfigure = ctu_configure,

    .fnInit = ctu_init_compiler,

    .fnParse = ctu_parse_file,
    .fnForward = ctu_forward_decls,
    .fnImport = ctu_process_imports,
    .fnCompile = ctu_compile_module,
};
