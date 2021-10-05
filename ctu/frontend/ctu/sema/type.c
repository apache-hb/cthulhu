#include "type.h"

static const type_t *compile_typename(sema_t *sema, ctu_t *ctu) {
    const type_t *type = get_type(sema, ctu->ident);
    if (type == NULL) {
        report(sema->reports, ERROR, ctu->node, "unable to resolve type name `%s`", ctu->ident);
        return type_poison("unresolved type");
    }
    return type;
}

static const type_t *compile_pointer(sema_t *sema, ctu_t *ctu) {
    const type_t *type = compile_type(sema, ctu->ptr);
    return type_ptr(type);
}

const type_t *compile_type(sema_t *sema, ctu_t *ctu) {
    switch (ctu->type) {
    case CTU_TYPENAME:
        return compile_typename(sema, ctu);
    case CTU_POINTER:
        return compile_pointer(sema, ctu);

    default:
        ctu_assert(sema->reports, "compile-type unknown type %d", ctu->type);
        return type_poison("unknown type");
    }
}
