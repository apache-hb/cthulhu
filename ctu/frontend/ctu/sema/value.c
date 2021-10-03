#include "value.h"
#include "expr.h"
#include "eval.h"

static void realise_value(sema_t *sema, lir_t *lir, ctu_t *ctu) {
    if (!stack_enter(sema, lir)) {
        return;
    }

    lir_t *init = compile_expr(sema, ctu->value);

    lir_value(sema->reports, lir, lir_type(init), init);

    stack_leave(sema, lir);
}

lir_t *compile_value(lir_t *lir) {
    state_t *ctx = lir->ctx;
    realise_value(ctx->sema, lir, ctx->ctu);
    return lir;
}

void build_value(sema_t *sema, lir_t *lir) {
    state_t *state = lir->ctx;
    realise_value(sema, lir, state->ctu);
}
