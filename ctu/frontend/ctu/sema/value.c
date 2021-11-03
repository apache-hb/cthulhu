#include "value.h"
#include "expr.h"
#include "type.h"
#include "attrib.h"
#include "sema.h"

static void realise_value(sema_t *sema, lir_t *lir, ctu_t *ctu) {
    if (!stack_enter(sema, lir)) {
        return;
    }

    if (!lir_is(lir, LIR_FORWARD)) {
        stack_leave(sema, lir);
        return;
    }

    const type_t *type = NULL;
    lir_t *init = NULL;

    if (ctu->kind) {
        type = compile_type(sema, ctu->kind);
    }

    if (ctu->value) {
        init = compile_expr(sema, ctu->value);

        if (type == NULL) {
            type = lir_type(init);
        } else {
            /* check conversion */
        }
    }

    if (is_void(type)) {
        report(sema->reports, ERROR, ctu->node, "cannot initialize variable `%s` with type `%s`",
            ctu->name, type_format(type)
        );
    }

    type = type_mut(type, ctu->mut);

    lir_value(sema->reports, lir, type, init);

    compile_attribs(sema, lir, ctu);

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

lir_t *local_value(sema_t *sema, ctu_t *ctu) {
    lir_t *init = ctu->value != NULL ? compile_expr(sema, ctu->value) : NULL;
    const type_t *type = type_mut(ctu->kind != NULL ? compile_type(sema, ctu->kind) : lir_type(init), ctu->mut);
    lir_t *cvt = NULL;
    if (init != NULL) {
        cvt = implicit_convert_expr(sema, init, type);
        if (cvt == NULL) {
            report(sema->reports, ERROR, ctu->node, "cannot initialize a local of type `%s` with an expression of type `%s`", 
                type_format(type),
                type_format(lir_type(init))
            );
        }
    }

    lir_t *lir = lir_local(ctu->node, ctu->name, type, init);

    return lir;
}
