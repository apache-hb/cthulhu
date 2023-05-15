#include "cthulhu/mediator/language.h"

typedef struct lang_handle_t
{
    lifetime_t *parent;
    const language_t *language;

    void *user;
} lang_handle_t;
