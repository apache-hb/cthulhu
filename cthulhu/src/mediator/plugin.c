#include "cthulhu/mediator/plugin.h"

#include "base/panic.h"

typedef struct plugin_handle_t
{
    lifetime_t *lifetime;
    const plugin_t *plugin;

    void *user;
} plugin_handle_t;

void plugin_set_user(plugin_handle_t *self, void *user)
{
    CTASSERT(self != NULL);

    self->user = user;
}

void *plugin_get_user(plugin_handle_t *self)
{
    CTASSERT(self != NULL);
    return self->user;
}
