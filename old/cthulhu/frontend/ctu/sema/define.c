#include "define.h"
#include "type.h"
#include "expr.h"
#include "attrib.h"

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

    lir_t *body = NULL;
    const type_t *type = is_poison(lir_type(lir))
        ? realise_closure(sema, ctu)
        : lir_type(lir);

    if (ctu->body != NULL) {
        size_t sizes[TAG_MAX] = {
            [TAG_GLOBALS] = MAP_SMALL,
            [TAG_FUNCS] = MAP_SMALL,
            [TAG_TYPES] = MAP_SMALL,
            [TAG_USERTYPES] = MAP_SMALL
        };

        sema_t *nest = new_sema(sema->reports, sema, sizes);

        set_return(nest, closure_result(type));
        add_locals(nest, type, ctu->params);

        body = compile_stmts(nest, ctu->body);

        sema_delete(nest);
    }

    lir_define(sema->reports, lir, 
        /* type = */ type,
        /* body = */ body
    );

    compile_attribs(sema, lir, ctu);

    if (ctu->body == NULL) {
        add_extern(sema, lir);
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
    const type_t *it = realise_closure(ctx->sema, ctx->ctu);
    retype_lir(lir, it);
    return lir;
}

void build_define(sema_t *sema, lir_t *lir) {
    state_t *state = lir->ctx;
    realise_define(sema, lir, state->ctu);
}

void add_locals(sema_t *sema, const type_t *type, vector_t *params) {
    size_t len = vector_len(params);
    for (size_t i = 0; i < len; i++) {
        ctu_t *param = vector_get(params, i);
        const type_t *arg = param_at(type, i);
        const char *name = param->name;
        if (!is_discard(name)) {
            add_var(sema, name, lir_param(param->node, param->name, arg, i));
        }
    }
}

const type_t *lambda_type(sema_t *sema, ctu_t *ctu) {
    return realise_closure(sema, ctu);
}