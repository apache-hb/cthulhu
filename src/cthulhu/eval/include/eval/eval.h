#pragma once

#include <ctu_eval_api.h>

BEGIN_API

typedef struct eval_t eval_t;
typedef struct arena_t arena_t;
typedef struct logger_t logger_t;

typedef struct eval_config_t
{
    arena_t *arena;
    logger_t *logger;
    size_t ops_limit;
} eval_config_t;

END_API
