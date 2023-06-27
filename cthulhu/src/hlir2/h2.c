#include "cthulhu/hlir2/h2.h"

#include "base/memory.h"

static h2_t *h2_new(h2_kind_t kind, const node_t *node, const h2_t *type)
{
    h2_t *self = ctu_malloc(sizeof(h2_t));

    self->kind = kind;
    self->node = node;
    self->type = type;

    return self;
}

h2_t *h2_error(const node_t *node, const char *message)
{
    h2_t *self = h2_new(eHlir2Error, node, NULL);

    self->message = message;

    return self;
}
