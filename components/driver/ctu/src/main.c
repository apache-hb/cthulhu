#include "cthulhu/interface/interface.h"
#include "cthulhu/mediator/mediator.h"
#include "cthulhu/interface/runtime.h"
#include "scan/compile.h"

#include "sema/sema.h"
#include "sema/config.h"

#include "base/macros.h"

#include "ctu-bison.h"
#include "ctu-flex.h"

CT_CALLBACKS(kCallbacks, ctu);

static void *ctu_parse_file(runtime_t *runtime, compile_t *compile)
{
    UNUSED(runtime);

    init_scan(compile->scan);
    return compile_scanner(compile->scan, &kCallbacks);
}

static const char *kLangNames[] = { ".ct", ".ctu", NULL };

static const driver_t kDriver = {
    .name = "Cthulhu",
    .version = NEW_VERSION(1, 0, 0),

    .exts = kLangNames,

    .fnInitCompiler = ctu_init_compiler,
    .fnParseFile = ctu_parse_file,
    .fnForwardDecls = ctu_forward_decls,
    .fnResolveImports = ctu_process_imports,
    .fnCompileModule = ctu_compile_module,
};

driver_t get_driver()
{
    return kDriver;
}

static void ctu_configure(lang_handle_t *handle, ap_t *ap)
{
    ctu_config_init(handle, ap);
}

static const language_t kLanguageInfo = {
    .id = "ctu",
    .name = "Cthulhu",
    .version = NEW_VERSION(1, 1, 0),

    .exts = kLangNames,

    .fnConfigure = ctu_configure,
};

LANGUAGE_EXPORT
extern const language_t *LANGUAGE_ENTRY_POINT(mediator_t *mediator)
{
    UNUSED(mediator);

    return &kLanguageInfo;
}
