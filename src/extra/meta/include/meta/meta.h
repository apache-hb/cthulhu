#pragma once

#include "core/compiler.h"

typedef struct meta_ast_t meta_ast_t;
typedef struct arena_t arena_t;
typedef struct io_t io_t;

CT_BEGIN_API

void meta_emit(meta_ast_t *ast, io_t *header, io_t *source, arena_t *arena);

CT_END_API
