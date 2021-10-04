#include "value.h"
#include "expr.h"
#include "eval.h"

#include "ctu/type/retype.h"

static const attrib_t EXPORTED = { .visibility = PUBLIC };

static void attach_attribs(lir_t *decl, ctu_t *ctu) {
    if (ctu->exported) {
        lir_attribs(decl, &EXPORTED);
    }
}

static void realise_value(sema_t *sema, lir_t *lir, ctu_t *ctu) {
    if (!stack_enter(sema, lir)) {
        return;
    }

    if (!lir_is(lir, LIR_FORWARD)) {
        stack_leave(sema, lir);
        return;
    }

    lir_t *expr = compile_expr(sema, ctu->value);
    lir_t *init = retype_expr(sema->reports, 
        types_common(lir_type(expr), type_digit(SIGNED, TY_INT)),
        expr
    );

    lir_value(sema->reports, lir, lir_type(init), init);

    attach_attribs(lir, ctu);

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
