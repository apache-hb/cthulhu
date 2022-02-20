#include "common.h"

const hlir_attributes_t DEFAULT_ATTRIBS = {
    .linkage = LINK_INTERNAL
};

hlir_t *hlir_new(const node_t *node, const hlir_t *of, hlir_type_t type) {
    hlir_t *self = ctu_malloc(sizeof(hlir_t));
    self->type = type;
    self->node = node;
    self->of = of;
    return self;
}

hlir_t *hlir_new_decl(const node_t *node, const char *name, const hlir_t *of, hlir_type_t type) {
    hlir_t *hlir = hlir_new(node, of, type);
    hlir->name = name;
    hlir->attributes = &DEFAULT_ATTRIBS;
    return hlir;
}

// accessors

const hlir_t *typeof_hlir(const hlir_t *hlir) {
    return hlir->of;
}

const char *nameof_hlir(const hlir_t *self) {
    return self->name;
}

bool hlir_is_imported(const hlir_t *self) {
    return self->attributes->linkage == LINK_IMPORTED;
}

bool hlir_is(const hlir_t *self, hlir_type_t type) {
    return self->type == type;
}

bool hlir_can_be(const hlir_t *self, hlir_type_t type) {
    return self->type == type || (self->type == HLIR_FORWARD && self->expected == type);
}

vector_t *closure_params(const hlir_t *self) {
    CTASSERT(self->type == HLIR_CLOSURE, "closure-params(self->type != HLIR_CLOSURE)");

    return self->params;
}

bool closure_variadic(const hlir_t *self) {
    CTASSERT(self->type == HLIR_CLOSURE, "closure-variadic(self->type != HLIR_CLOSURE)");

    return self->variadic;
}

const hlir_t *closure_result(const hlir_t *self) {
    CTASSERT(self->type == HLIR_CLOSURE, "closure-result(self->type != HLIR_CLOSURE)");

    return self->result;
}
