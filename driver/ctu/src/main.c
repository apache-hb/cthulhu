#include "cthulhu/interface/interface.h"
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

static const driver_t kBaseDriver = {
    .name = "Cthulhu",
    .version = NEW_VERSION(1, 0, 0),

    .exts = ".ct,.ctu",

    .fnInitCompiler = ctu_init_compiler,
    .fnParseFile = ctu_parse_file,
    .fnForwardDecls = ctu_forward_decls,
    .fnResolveImports = ctu_process_imports,
    .fnCompileModule = ctu_compile_module,
};

driver_t get_driver()
{
    driver_t result = kBaseDriver;

    result.commandGroups = config_get_groups();

    return result;
}
