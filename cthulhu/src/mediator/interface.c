#include "common.h"

#include "base/memory.h"
#include "base/panic.h"

mediator_t *mediator_new(const char *id, version_info_t version)
{
    CTASSERT(id != NULL);

    mediator_t *self = ctu_malloc(sizeof(mediator_t));

    self->id = id;
    self->version = version;

    return self;
}

lifetime_t *lifetime_new(mediator_t *mediator)
{
    CTASSERT(mediator != NULL);

    lifetime_t *self = ctu_malloc(sizeof(lifetime_t));

    self->parent = mediator;

    return self;
}

context_t *context_new(lifetime_t *lifetime)
{
    CTASSERT(lifetime != NULL);
    
    context_t *self = ctu_malloc(sizeof(context_t));

    self->parent = lifetime;

    return self;
}
