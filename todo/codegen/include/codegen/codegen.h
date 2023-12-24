#pragma once

typedef struct arena_t arena_t;
typedef struct io_t io_t;

typedef struct cg_context_t cg_context_t;
typedef struct cg_class_t cg_class_t;

cg_context_t *cg_context_new(const char *prefix, arena_t *arena);

cg_class_t *cg_root_class(cg_context_t *ctx);
cg_class_t *cg_class_new(const char *name, cg_class_t *super);
void cg_class_add_basic_field(cg_class_t *class, const char *name, const char *type);
void cg_class_add_class_field(cg_class_t *class, const char *name, const cg_class_t *type);

void cg_emit_header(cg_context_t *ctx, io_t *io);
