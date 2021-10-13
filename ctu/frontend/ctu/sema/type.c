#include "type.h"

static type_t *compile_typename(sema_t *sema, ctu_t *ctu) {
    type_t *type = get_type(sema, ctu->ident);
    if (type == NULL) {
        report(sema->reports, ERROR, ctu->node, "unable to resolve type name `%s`", ctu->ident);
        return type_poison_with_node("unresolved type", ctu->node);
    }
    return type;
}

static type_t *compile_pointer(sema_t *sema, ctu_t *ctu) {
    type_t *type = compile_type(sema, ctu->ptr);
    return type_ptr(type);
}

static type_t *compile_closure(sema_t *sema, ctu_t *ctu) {
    type_t *result = compile_type(sema, ctu->result);
    
    size_t len = vector_len(ctu->params);
    vector_t *args = vector_of(len);

    for (size_t i = 0; i < len; i++) {
        ctu_t *param = vector_get(ctu->params, i);
        type_t *type = compile_type(sema, param);
        vector_set(args, i, type);
    }

    return type_closure(args, result);
}

static type_t *compile_mutable(sema_t *sema, ctu_t *ctu) {
    type_t *type = compile_type(sema, ctu->kind);

    if (!is_const(type)) {
        message_t *id = report(sema->reports, WARNING, ctu->node, "type is already mutable");
        report_underline(id, "redundant mutable specifier");
    }

    return type_mut(type, true);
}

type_t *compile_type(sema_t *sema, ctu_t *ctu) {
    switch (ctu->type) {
    case CTU_TYPENAME:
        return compile_typename(sema, ctu);
    case CTU_POINTER:
        return compile_pointer(sema, ctu);
    case CTU_CLOSURE:
        return compile_closure(sema, ctu);
    case CTU_MUTABLE:
        return compile_mutable(sema, ctu);

    default:
        ctu_assert(sema->reports, "compile-type unknown type %d", ctu->type);
        return type_poison_with_node("unknown type", ctu->node);
    }
}
