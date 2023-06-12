#pragma once

#include "cthulhu/mediator/driver.h"

typedef struct langs_t {
    const language_t *langs;
    size_t size;
} langs_t;

langs_t get_langs(void);
