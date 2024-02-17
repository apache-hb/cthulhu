#pragma once

#include "core/compiler.h"

#include <stdarg.h>

typedef struct arena_t arena_t;
typedef struct io_t io_t;

CT_BEGIN_API

typedef struct cxx_emit_t cxx_emit_t;

cxx_emit_t *cxx_emit_new(arena_t *arena);

void cxx_enter(cxx_emit_t *emit);
void cxx_leave(cxx_emit_t *emit);

void cxx_indent(cxx_emit_t *emit);

void cxx_vwriteln(cxx_emit_t *emit, const char *fmt, va_list args);
void cxx_vwrite(cxx_emit_t *emit, const char *fmt, va_list args);

void cxx_writeln(cxx_emit_t *emit, const char *fmt, ...);
void cxx_write(cxx_emit_t *emit, const char *fmt, ...);
void cxx_nl(cxx_emit_t *emit);

void cxx_close_brace(cxx_emit_t *emit);

void cxx_privacy(cxx_emit_t *emit, const char *privacy);

void cxx_dump(cxx_emit_t *emit, io_t *io);

CT_END_API
