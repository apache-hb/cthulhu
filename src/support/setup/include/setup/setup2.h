#pragma once

#include <ctu_setup_api.h>

typedef struct arena_t arena_t;
typedef struct io_t io_t;

CT_BEGIN_API

typedef struct setup_t
{
    arena_t *arena;

    io_t *io;
} setup_t;

CT_END_API
