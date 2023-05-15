#include "cthulhu/mediator/mediator.h"

#include "std/vector.h"

#include "io/io.h"

#include "base/util.h"
#include "base/panic.h"
#include "base/memory.h"

typedef struct lifetime_t
{
    mediator_t *parent;
    vector_t *files; /// vector_t<source_t>
} lifetime_t;

lifetime_t *mediator_get_lifetime(mediator_t *self)
{
    CTASSERT(self != NULL);

    lifetime_t *lifetime = ctu_malloc(sizeof(lifetime_t));
    lifetime->parent = self;
    lifetime->files = vector_new(4);
    return lifetime;
}

void lifetime_add_source(lifetime_t *self, source_t source)
{
    CTASSERT(self != NULL);
    CTASSERT(io_error(source.io) == 0);
    CTASSERT(source.lang != NULL);

    vector_push(&self->files, BOX(source));
}
