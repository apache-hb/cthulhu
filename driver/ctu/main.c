#include "cthulhu/ast/compile.h"
#include "cthulhu/interface/interface.h"
#include "cthulhu/interface/runtime.h"

#include "sema.h"

#include "ctu-bison.h"
#include "ctu-flex.h"

#include <malloc.h>

CT_CALLBACKS(kCallbacks, ctu);

static void *ctu_parse_file(runtime_t *runtime, compile_t *compile)
{
    UNUSED(runtime);

    init_scan(compile->scanner);
    ast_t *ast = compile_file(compile->scanner, &kCallbacks);

    logverbose("[parse-file] ast: %p", ast);
    logverbose("[parse-file] decls: %p", ast->decls);

    ast_t *ast2 = program;

    logverbose("[parse-file] offsets: %zu %zu %zu", offsetof(ast_t, modspec), offsetof(ast_t, imports), offsetof(ast_t, decls));
    logverbose("[parse-file] actual: (%p, %p)", ast, ast2);
    logverbose("[parse-file] modspec: (%p)", ast->modspec);
    logverbose("[parse-file] modspec2: (%p)", ast2->modspec);

    logverbose("[parse-file] imports: (%p)", ast->imports);
    logverbose("[parse-file] imports2: (%p)", ast2->imports);

    logverbose("[parse-file] decls: (%p)", ast->decls);
    logverbose("[parse-file] decls2: (%p)", ast2->decls);

    return ast;
}

const driver_t kDriver = {
    .name = "Cthulhu",
    .version = NEW_VERSION(1, 0, 0),

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
