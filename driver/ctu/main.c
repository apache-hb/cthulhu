#include "cthulhu/ast/compile.h"
#include "cthulhu/driver/driver.h"

#include "sema.h"

#include "ctu-bison.h"
#include "ctu-flex.h"

CT_CALLBACKS(kCallbacks, ctu);

void ctu_parse_file(runtime_t *runtime, compile_t *compile)
{
    UNUSED(runtime);
    compile->ast = compile_file(compile->scanner, &kCallbacks);
}

static const driver_t kDriver = {
    .name = "Cthulhu",
    .version = NEW_VERSION(1, 0, 0),
    .fnParseFile = ctu_parse_file,
    .fnForwardDecls = ctu_forward_decls,
    .fnResolveImports = ctu_process_imports,
    .fnCompileModule = ctu_compile_module,
};

int main(int argc, const char **argv)
{
    common_init();

    return common_main(argc, argv, kDriver);
}
