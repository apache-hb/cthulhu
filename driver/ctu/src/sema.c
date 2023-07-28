#include "ctu/sema.h"
#include "ctu/ast.h"

#include "base/macros.h"

#include "cthulhu/mediator/driver.h"

#include "cthulhu/hlir/h2.h"

void ctu_forward_decls(context_t *context)
{
    lifetime_t *lifetime = context_get_lifetime(context);
    ast_t *ast = context_get_ast(context);
    const char *name = context_get_name(context);
    reports_t *reports = lifetime_get_reports(lifetime);

    size_t sizes[eTagTotal] = { 0 };

    h2_t *mod = h2_module_root(reports, ast->node, name, eTagTotal, sizes);

    context_update(context, ast, mod);
}

void ctu_process_imports(context_t *context)
{
    CTU_UNUSED(context);
}

void ctu_compile_module(context_t *context)
{
    CTU_UNUSED(context);
}
