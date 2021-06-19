#include "c.h"

static void c_compile_flow(c_ctx_t *ctx, flow_t *flow) {
    fprintf(ctx->file, "int64_t %s(void) {\n", flow->name);
    fprintf(ctx->file, "  return 0;\n");
    fprintf(ctx->file, "}\n\n");
}

c_ctx_t c_compile(unit_t *unit, FILE *file) {
    c_ctx_t ctx = { file };

    fprintf(ctx.file, "#include <stdint.h>\n\n");

    for (size_t i = 0; i < unit->len; i++) {
        c_compile_flow(&ctx, unit->flows + i);
    }

    return ctx;
}

void c_output(c_ctx_t *ctx) {
    fclose(ctx->file);
}
