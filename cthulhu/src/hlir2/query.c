#include "common.h"

const node_t *h2_get_node(const h2_t *self)
{
    return self->node;
}

const char *h2_get_name(const h2_t *self)
{
    return self->name;
}

bool h2_is(const h2_t *self, h2_kind_t kind)
{
    return self->kind == kind;
}

bool h2_has_quals(const h2_t *self, h2_quals_t quals)
{
    if (h2_is(self, eHlir2Qualify))
    {
        return self->quals & quals;
    }

    return false;
}

bool h2_has_vis(const h2_t *self, h2_visible_t visible)
{
    return self->attrib->visible == visible;
}
