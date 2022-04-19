#include "common.h"

#include "cthulhu/hlir/query.h"

const hlir_attributes_t DEFAULT_ATTRIBS = {
    .linkage = LINK_INTERNAL
};

hlir_t *TYPE = NULL;
hlir_t *INVALID = NULL;

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
    hlir->attributes = &DEFAULT_ATTRIBS;
    return hlir;
}

hlir_t *hlir_new_forward(const node_t *node, const char *name, const hlir_t *of, hlir_kind_t expect) {
    hlir_t *hlir = hlir_new_decl(node, name, of, HLIR_FORWARD);
    hlir->expected = expect;
    return hlir;
}

void init_hlir(void) {
    TYPE = hlir_new(NULL, NULL, HLIR_TYPE);
    TYPE->of = TYPE;
    TYPE->name = "type";

    INVALID = hlir_error(NULL, "invalid hlir node");
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

static const char *SIGN_NAMES[SIGN_TOTAL] = {
    [SIGN_DEFAULT] = "default",
    [SIGN_UNSIGNED] = "unsigned",
    [SIGN_SIGNED] = "signed"
};

static const char *DIGIT_NAMES[DIGIT_TOTAL] = {
    [DIGIT_CHAR] = "char",
    [DIGIT_SHORT] = "short",
    [DIGIT_INT] = "int",
    [DIGIT_LONG] = "long",
    [DIGIT_SIZE] = "size",
    [DIGIT_PTR] = "intptr"
};

const char *sign_name(sign_t sign) {
    return SIGN_NAMES[sign];
}

const char *digit_name(digit_t digit) {
    return DIGIT_NAMES[digit];
}
