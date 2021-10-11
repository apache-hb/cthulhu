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

type_t *compile_type(sema_t *sema, ctu_t *ctu) {
    switch (ctu->type) {
    case CTU_TYPENAME:
        return compile_typename(sema, ctu);
    case CTU_POINTER:
        return compile_pointer(sema, ctu);

    default:
        ctu_assert(sema->reports, "compile-type unknown type %d", ctu->type);
        return type_poison_with_node("unknown type", ctu->node);
    }
}

void add_struct(sema_t *sema, ctu_t *ctu) {
    const char *name = ctu->name;
    const type_t *type = get_type(sema, name);
    if (type != NULL) {
        report(sema->reports, ERROR, ctu->node, "struct `%s` shadows an existing type", ctu->name);
    }

    vector_t *fields = ctu->fields;
    size_t num_fields = vector_len(fields);

    vector_t *all_fields = vector_of(num_fields);
    for (size_t i = 0; i < num_fields; i++) {
        ctu_t *field_node = vector_get(fields, i);
        type_t *field_type = compile_type(sema, field_node->kind);

        aggregate_field_t *field = new_aggregate_field(field_node->name, field_type);

        vector_set(all_fields, i, field);
    }

    type_t *struct_type = type_struct(name, ctu->node, all_fields);

    add_type(sema, name, struct_type);
}

void add_union(sema_t *sema, ctu_t *ctu) {
    const char *name = ctu->name;
    const type_t *type = get_type(sema, name);
    if (type != NULL) {
        report(sema->reports, ERROR, ctu->node, "struct `%s` shadows an existing type", ctu->name);
    }

    add_type(sema, name, type_poison("add-union unimplemented"));
}
