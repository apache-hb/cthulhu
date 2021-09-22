#include "c89.h"

#include "ctu/util/str.h"

typedef struct {
    reports_t *reports;
    module_t *mod;

    vector_t *globals;
} context_t;

static context_t *init_c89_context(reports_t *reports, module_t *mod) {
    context_t *ctx = NEW(context_t);
    ctx->reports = reports;
    ctx->mod = mod;
    ctx->globals = vector_new(vector_len(mod->vars));
    return ctx;
}

bool c89_build(reports_t *reports, module_t *mod, const char *path) {
    assert2(reports, "c89 unimplemented %p %s", mod, path);
    return false;
}
