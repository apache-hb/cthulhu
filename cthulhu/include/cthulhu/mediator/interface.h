#pragma once

#include "cthulhu/mediator/common.h"

// mediator api

mediator_t *mediator_new(const char *id, version_info_t version);

void mediator_add_language(mediator_t *mediator, const language_t *lang);


// lifetime api

lifetime_t *lifetime_new(mediator_t *mediator);


// context api

context_t *context_new(lifetime_t *lifetime);
