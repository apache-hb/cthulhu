#pragma once

#include <ctu_json_api.h>

typedef struct io_t io_t;
typedef struct logger_t logger_t;
typedef struct arena_t arena_t;
typedef struct json_ast_t json_ast_t;

CT_BEGIN_API

json_ast_t *json_scan(io_t *io, logger_t *logger, arena_t *arena);

CT_END_API
