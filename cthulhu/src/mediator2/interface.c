#include "common.h"

#include "base/memory.h"

mediator_t *mediator_new(const char *id, version_info_t version)
{
    mediator_t *self = ctu_malloc(sizeof(mediator_t));

    self->id = id;
    self->version = version;

    return self;
}

lifetime_t *lifetime_new(mediator_t *mediator)
{
    lifetime_t *self = ctu_malloc(sizeof(lifetime_t));

    self->parent = mediator;

    return self;
}

context_t *context_new(lifetime_t *lifetime)
{
    context_t *self = ctu_malloc(sizeof(context_t));

    self->parent = lifetime;

    return self;
}
