#include "cthulhu/interface/interface.h"
#include "cthulhu/interface/runtime.h"
#include "scan/compile.h"

#include "sema.h"

#include "base/macros.h"

#include "pl0-bison.h"
#include "pl0-flex.h"

CT_CALLBACKS(kCallbacks, pl0);

static void *pl0_parse_file(runtime_t *runtime, compile_t *compile)
{
    UNUSED(runtime);

    return compile_scanner(compile->scanner, &kCallbacks);
}

const driver_t kDriver = {
    .name = "PL/0",
    .version = NEW_VERSION(2, 2, 0),

    .fnInitCompiler = pl0_init,
    .fnParseFile = pl0_parse_file,
    .fnForwardDecls = pl0_forward_decls,
    .fnResolveImports = pl0_process_imports,
    .fnCompileModule = pl0_compile_module,
};

driver_t get_driver(void)
{
    return kDriver;
}
