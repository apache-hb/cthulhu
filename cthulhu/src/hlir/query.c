#include "cthulhu/hlir/query.h"

#include "std/str.h"

#include "base/panic.h"

static bool has_name(h2_kind_t kind)
{
    switch (kind)
    {
    case eHlir2TypeEmpty:
    case eHlir2TypeUnit:
    case eHlir2TypeBool:
    case eHlir2TypeDigit:
    case eHlir2TypeString:
    case eHlir2TypeClosure:

    case eHlir2DeclGlobal:
    case eHlir2DeclLocal:
    case eHlir2DeclParam:
    case eHlir2DeclFunction:
    case eHlir2DeclModule:
        return true;

    default:
        return false;
    }
}

static const char *h2_kind_to_string(h2_kind_t kind)
{
    switch (kind)
    {
#define HLIR_KIND(ID, NAME) case ID: return NAME;
#include "cthulhu/hlir/hlir.inc"

    default: NEVER("invalid hlir kind %d", kind);
    }
}

const char *h2_to_string(const h2_t *self)
{
    if (self == NULL) { return "nil"; }

    if (h2_is(self, eHlir2Error))
    {
        return format("{ error: %s }", self->message);
    }

    if (has_name(self->kind))
    {
        return format("{ %s: %s }", h2_kind_to_string(self->kind), h2_get_name(self));
    }

    return h2_kind_to_string(self->kind);
}

const node_t *h2_get_node(const h2_t *self)
{
    CTASSERT(self != NULL);

    return self->node;
}

const char *h2_get_name(const h2_t *self)
{
    CTASSERT(self != NULL);

    return self->name;
}

h2_kind_t h2_get_kind(const h2_t *self)
{
    CTASSERT(self != NULL);

    return self->kind;
}

const h2_t *h2_get_type(const h2_t *self)
{
    CTASSERT(self != NULL);

    return self->type;
}

const h2_attrib_t *h2_get_attrib(const h2_t *self)
{
    CTASSERT(self != NULL);

    return self->attrib;
}

bool h2_is(const h2_t *self, h2_kind_t kind)
{
    CTASSERT(self != NULL);

    return self->kind == kind;
}

bool h2_has_quals(const h2_t *self, quals_t quals)
{
    if (h2_is(self, eHlir2Qualify))
    {
        return self->quals & quals;
    }

    return false;
}

bool h2_has_vis(const h2_t *self, h2_visible_t visibility)
{
    const h2_attrib_t *attrib = h2_get_attrib(self);
    return attrib->visibility == visibility;
}
