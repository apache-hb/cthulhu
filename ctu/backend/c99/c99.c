#include "c99.h"

#include "ctu/util/str.h"

typedef struct {
    reports_t *reports;
    module_t *mod;

    vector_t *globals;

    stream_t *result;
} context_t;

static context_t *init_c99_context(reports_t *reports, module_t *mod) {
    context_t *ctx = NEW(context_t);
    ctx->reports = reports;
    ctx->mod = mod;
    ctx->globals = vector_new(vector_len(mod->vars));
    ctx->result = stream_new(0x1000);
    return ctx;
}

bool c99_build(reports_t *reports, module_t *mod, const char *path) {
    context_t *ctx = init_c99_context(reports, mod);
    
    printf("%s\n", stream_data(ctx->result));

    assert2(reports, "c99 unimplemented %p %s", mod, path);
    return false;
}
