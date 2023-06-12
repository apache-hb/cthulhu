#pragma once

#include "cthulhu/mediator2/common.h"

typedef struct mediator_t
{
    const char *id;
    version_info_t version;
} mediator_t;

typedef struct lifetime_t 
{
    mediator_t *parent;
} lifetime_t;

typedef struct context_t 
{
    lifetime_t *parent;
} context_t;
