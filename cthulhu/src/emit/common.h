#pragma once

#include <stddef.h>

typedef struct reports_t reports_t;
typedef struct io_t io_t;

typedef struct emit_t 
{
    io_t *io;

    size_t indent;
} emit_t;

void write_string(emit_t *emit, const char *str);

void emit_indent(emit_t *emit);
void emit_dedent(emit_t *emit);

#define WRITE_STRINGF(emit, str, ...) write_string(emit, format(str, __VA_ARGS__))
#define WRITE_STRING(emit, str) write_string(emit, str)
