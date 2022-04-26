#include "common.h"

#include "cthulhu/hlir/query.h"

static const hlir_attributes_t kDefaultAttributes = { .linkage = LINK_INTERNAL, .tags = DEFAULT_TAGS };

hlir_t *kMetaType = NULL;
hlir_t *kInvalidNode = NULL;

hlir_t *hlir_new(const node_t *node, const hlir_t *of, hlir_kind_t kind) {
    hlir_t *self = ctu_malloc(sizeof(hlir_t));
    self->type = kind;
    self->node = node;
    self->of = of;
    return self;
}

hlir_t *hlir_new_decl(const node_t *node, const char *name, const hlir_t *of, hlir_kind_t kind) {
    hlir_t *hlir = hlir_new(node, of, kind);
    hlir->name = name;
    hlir->attributes = &kDefaultAttributes;
    return hlir;
}

hlir_t *hlir_new_forward(const node_t *node, const char *name, const hlir_t *of, hlir_kind_t expect) {
    hlir_t *hlir = hlir_new_decl(node, name, of, HLIR_FORWARD);
    hlir->expected = expect;
    return hlir;
}

void init_hlir(void) {
    kMetaType = hlir_new(NULL, NULL, HLIR_TYPE);
    kMetaType->of = kMetaType;
    kMetaType->name = "type";

    kInvalidNode = hlir_error(NULL, "invalid hlir node");
}

// accessors

bool hlir_is_imported(const hlir_t *self) {
    return self->attributes->linkage == LINK_IMPORTED;
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
