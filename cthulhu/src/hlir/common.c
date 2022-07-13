#include "common.h"

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/query.h"
#include "scan/node.h"

#include "base/macros.h"
#include "base/memory.h"
#include "base/panic.h"

static const hlir_attributes_t kDefaultAttributes = {.linkage = eLinkInternal, .tags = DEFAULT_TAGS};

hlir_t *kMetaType = NULL;
hlir_t *kInvalidNode = NULL;

hlir_t *hlir_new(node_t node, const hlir_t *of, hlir_kind_t kind)
{
    hlir_t *self = ctu_malloc(sizeof(hlir_t));
    self->type = kind;
    self->location = node;
    self->of = of;
    return self;
}

hlir_t *hlir_new_decl(node_t node, const char *name, const hlir_t *of, hlir_kind_t kind)
{
    hlir_t *hlir = hlir_new(node, of, kind);
    hlir->name = name;
    hlir->attributes = &kDefaultAttributes;
    hlir->parentDecl = NULL;
    return hlir;
}

hlir_t *hlir_new_forward(node_t node, const char *name, const hlir_t *of, hlir_kind_t expect)
{
    hlir_t *hlir = hlir_new_decl(node, name, of, eHlirForward);
    hlir->expected = expect;
    return hlir;
}

void init_hlir(void)
{
    kMetaType = hlir_new(node_builtin(), NULL, eHlirType);
    kMetaType->of = kMetaType;
    kMetaType->name = "type";

    kInvalidNode = hlir_error(node_builtin(), "invalid hlir node");
}

// accessors

bool hlir_is_imported(const hlir_t *self)
{
    return get_hlir_attributes(self)->linkage == eLinkImported;
}

bool hlir_is_exported(const hlir_t *self)
{
    return get_hlir_attributes(self)->linkage == eLinkExported;
}

static bool is_signature(const hlir_t *hlir)
{
    hlir_kind_t kind = get_hlir_kind(hlir);
    return kind == eHlirClosure || kind == eHlirFunction || hlir_will_be(hlir, eHlirFunction);
}

#define ENSURE_VALID_CLOSURE(hlir, str)                                                                                \
    CTASSERTF(is_signature(hlir), str "(%s)", hlir_kind_to_string(get_hlir_kind(hlir)))

vector_t *closure_params(const hlir_t *self)
{
    ENSURE_VALID_CLOSURE(self, "closure-params");

    return self->params;
}

bool closure_variadic(const hlir_t *self)
{
    ENSURE_VALID_CLOSURE(self, "closure-variadic");

    return self->variadic;
}

const hlir_t *closure_result(const hlir_t *self)
{
    ENSURE_VALID_CLOSURE(self, "closure-result");

    return self->result;
}
