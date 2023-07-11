#include "common.h"

#include "cthulhu/hlir/query.h"

const char *h2_to_string(const h2_t *self)
{
    if (self == NULL) { return "nil"; }

    return h2_get_name(self);
}

const node_t *h2_get_node(const h2_t *self)
{
    return self->node;
}

const char *h2_get_name(const h2_t *self)
{
    return self->name;
}

h2_kind_t h2_get_kind(const h2_t *self)
{
    return self->kind;
}

const h2_t *h2_get_type(const h2_t *self)
{
    return self->type;
}

const h2_attrib_t *h2_get_attrib(const h2_t *self)
{
    return self->attrib;
}

bool h2_is(const h2_t *self, h2_kind_t kind)
{
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
    return self->attrib->visibility == visibility;
}
