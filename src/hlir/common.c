#include "common.h"

#include "cthulhu/ast/ast.h"
#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/query.h"

#include "cthulhu/util/util.h"

static const hlir_attributes_t kDefaultAttributes = {.linkage = LINK_INTERNAL, .tags = DEFAULT_TAGS};

hlir_t *kMetaType = NULL;
hlir_t *kInvalidNode = NULL;

hlir_t *hlir_new(const node_t *node, const hlir_t *of, hlir_kind_t kind)
{
    CTASSERTF(node != NULL, "hlir location for %s must be non-NULL", hlir_kind_to_string(kind));

    hlir_t *self = ctu_malloc(sizeof(hlir_t));
    self->type = kind;
    self->location = node;
    self->of = of;
    return self;
}

hlir_t *hlir_new_decl(const node_t *node, const char *name, const hlir_t *of, hlir_kind_t kind)
{
    hlir_t *hlir = hlir_new(node, of, kind);
    hlir->name = name;
    hlir->attributes = &kDefaultAttributes;
    hlir->parentDecl = NULL;
    return hlir;
}

hlir_t *hlir_new_forward(const node_t *node, const char *name, const hlir_t *of, hlir_kind_t expect)
{
    hlir_t *hlir = hlir_new_decl(node, name, of, HLIR_FORWARD);
    hlir->expected = expect;
    return hlir;
}

void init_hlir(void)
{
    kMetaType = hlir_new(node_builtin(), NULL, HLIR_TYPE);
    kMetaType->of = kMetaType;
    kMetaType->name = "type";

    kInvalidNode = hlir_error(node_builtin(), "invalid hlir node");
}

// accessors

bool hlir_is_imported(const hlir_t *self)
{
    return self->attributes->linkage == LINK_IMPORTED;
}

#if ENABLE_DEBUG
static bool is_signature(const hlir_kind_t kind)
{
    return kind == HLIR_CLOSURE || kind == HLIR_FUNCTION;
}
#endif

vector_t *closure_params(const hlir_t *self)
{
#if ENABLE_DEBUG
    hlir_kind_t kind = get_hlir_kind(self);
    CTASSERTF(is_signature(kind), "closure-params(%s)", hlir_kind_to_string(kind));
#endif

    return self->params;
}

bool closure_variadic(const hlir_t *self)
{
#if ENABLE_DEBUG
    hlir_kind_t kind = get_hlir_kind(self);
    CTASSERTF(is_signature(kind), "closure-variadic(%s)", hlir_kind_to_string(kind));
#endif

    return self->variadic;
}

const hlir_t *closure_result(const hlir_t *self)
{
#if ENABLE_DEBUG
    hlir_kind_t kind = get_hlir_kind(self);
    CTASSERTF(is_signature(kind), "closure-result(%s)", hlir_kind_to_string(kind));
#endif

    return self->result;
}
