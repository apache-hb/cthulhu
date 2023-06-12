#pragma once

#include "cthulhu/mediator2/common.h"

mediator_t *mediator_new(const char *id, version_info_t version);

lifetime_t *lifetime_new(mediator_t *mediator);

context_t *context_new(lifetime_t *lifetime);
