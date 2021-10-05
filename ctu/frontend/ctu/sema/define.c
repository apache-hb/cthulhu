#include "define.h"
#include "type.h"

static const type_t *realise_closure(sema_t *sema, ctu_t *ctu) {
    vector_t *params = ctu->params;

    type_t *result = compile_type(sema, ctu->result);

    size_t len = vector_len(params);
    vector_t *args = vector_of(len);
    for (size_t i = 0; i < len; i++) {
        ctu_t *param = vector_get(params, i);
        type_t *it = compile_type(sema, param->kind);
        vector_set(args, i, it);
    }

    return type_closure(args, result);
}

static void realise_define(sema_t *sema, lir_t *lir, ctu_t *ctu) {
    if (!lir_is(lir, LIR_FORWARD)) {
        return;
    }

    
}

lir_t *compile_define(lir_t *lir) {
    if (!lir_is(lir, LIR_FORWARD)) {
        return lir;
    }

    const type_t *type = lir_type(lir);
    if (!is_poison(type)) {
        return lir;
    }
    
    state_t *ctx = lir->ctx;
    lir->type = realise_closure(ctx->sema, ctx->ctu);
    return lir;
}

void build_define(sema_t *sema, lir_t *lir) {
    state_t *state = lir->ctx;
    realise_define(sema, lir, state->ctu);
}
