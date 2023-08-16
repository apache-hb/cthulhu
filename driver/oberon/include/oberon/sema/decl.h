#pragma once

#include "oberon/sema/sema.h"

typedef struct obr_forward_t {
    obr_tags_t tag;
    h2_t *decl;
} obr_forward_t;

obr_forward_t obr_forward_decl(h2_t *sema, obr_t *decl);
