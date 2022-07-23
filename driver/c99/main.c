#include "cthulhu/interface/interface.h"
#include "cthulhu/interface/runtime.h"
#include "scan/compile.h"

#include "base/macros.h"

#include "cc-bison.h"
#include "cc-flex.h"

CT_CALLBACKS(kCallbacks, cc);

static void *pl0_parse_file(runtime_t *runtime, compile_t *compile)
{
    UNUSED(runtime);

    return compile_scanner(compile->scan, &kCallbacks);
}

const driver_t kDriver = {
    .name = "C99",
    .version = NEW_VERSION(2, 2, 0),

    .fnInitCompiler = cc_init,
    .fnParseFile = cc_parse_file,
    .fnForwardDecls = cc_forward_decls,
    .fnResolveImports = cc_process_imports,
    .fnCompileModule = cc_compile_module,
};

driver_t get_driver(void)
{
    return kDriver;
}
