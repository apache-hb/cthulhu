#include "value.h"
#include "expr.h"
#include "type.h"

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

    const type_t *type = type_any();
    lir_t *init = NULL;

    if (ctu->kind) {
        type = compile_type(sema, ctu->kind);
    }

    if (ctu->value) {
        lir_t *expr = compile_expr(sema, ctu->value);
        init = retype_expr(sema->reports, 
            types_common(lir_type(expr), type),
            expr
        );

        if (type == type_any()) {
            type = lir_type(init);
        }
    }

    if (is_void(type)) {
        report(sema->reports, ERROR, ctu->node, "cannot initialize variable `%s` with type `%s`",
            ctu->name, type_format(type)
        );
    }

    lir_value(sema->reports, lir, type, init);

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

lir_t *local_value(sema_t *sema, ctu_t *ctu) {
    lir_t *value = lir_forward(ctu->node, ctu->name, LIR_VALUE, state_new(sema, ctu));
    realise_value(sema, value, ctu);
    return value;
}
