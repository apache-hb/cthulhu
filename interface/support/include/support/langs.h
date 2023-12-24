#pragma once

#include <stddef.h>

#include "cthulhu/mediator/common.h"

BEGIN_API

typedef struct langs_t
{
    const language_t *langs;
    size_t size;
} langs_t;

langs_t get_langs(void);

END_API
