#include "ref/emit.h"

#include "arena/arena.h"

#include "base/panic.h"

#include "io/io.h"
#include "std/str.h"
#include "std/typed/vector.h"

typedef struct cxx_emit_t
{
    arena_t *arena;
    typevec_t *buffer;
    size_t depth;
} cxx_emit_t;

cxx_emit_t *cxx_emit_new(arena_t *arena)
{
    CTASSERT(arena != NULL);

    cxx_emit_t *emit = ARENA_MALLOC(sizeof(arena_t), "cxx_emit", NULL, arena);
    emit->arena = arena;
    emit->buffer = typevec_new(sizeof(char), 0x1000, arena);
    emit->depth = 0;
    return emit;
}

void cxx_enter(cxx_emit_t *emit)
{
    CTASSERT(emit != NULL);
    emit->depth += 1;
}

void cxx_leave(cxx_emit_t *emit)
{
    CTASSERT(emit != NULL);
    CTASSERT(emit->depth > 0);
    emit->depth -= 1;
}

void cxx_indent(cxx_emit_t *emit)
{
    CTASSERT(emit != NULL);
    for (size_t i = 0; i < emit->depth; i++)
    {
        typevec_append(emit->buffer, "    ", 4);
    }
}

void cxx_vwriteln(cxx_emit_t *emit, const char *fmt, va_list args)
{
    CTASSERT(emit != NULL);
    CTASSERT(fmt != NULL);

    cxx_indent(emit);
    cxx_vwrite(emit, fmt, args);
    cxx_nl(emit);
}

void cxx_vwrite(cxx_emit_t *emit, const char *fmt, va_list args)
{
    CTASSERT(emit != NULL);
    CTASSERT(fmt != NULL);

    text_t text = text_vformat(emit->arena, fmt, args);
    typevec_append(emit->buffer, text.text, text.length);
}

void cxx_writeln(cxx_emit_t *emit, const char *fmt, ...)
{
    CTASSERT(emit != NULL);
    CTASSERT(fmt != NULL);

    va_list args;
    va_start(args, fmt);
    cxx_vwriteln(emit, fmt, args);
    va_end(args);
}

void cxx_write(cxx_emit_t *emit, const char *fmt, ...)
{
    CTASSERT(emit != NULL);
    CTASSERT(fmt != NULL);

    va_list args;
    va_start(args, fmt);
    cxx_vwrite(emit, fmt, args);
    va_end(args);
}

void cxx_nl(cxx_emit_t *emit)
{
    CTASSERT(emit != NULL);
    typevec_append(emit->buffer, "\n", 1);
}

void cxx_close_brace(cxx_emit_t *emit)
{
    CTASSERT(emit != NULL);
    cxx_leave(emit);
    cxx_indent(emit);
    cxx_write(emit, "}\n");
}

void cxx_privacy(cxx_emit_t *emit, const char *privacy)
{
    CTASSERT(emit != NULL);
    CTASSERT(privacy != NULL);

    cxx_leave(emit);
    cxx_indent(emit);
    cxx_write(emit, "%s:\n", privacy);
    cxx_enter(emit);
}

void cxx_dump(cxx_emit_t *emit, io_t *io)
{
    CTASSERT(emit != NULL);
    CTASSERT(io != NULL);

    typevec_append(emit->buffer, "\0", 1);
    const char *data = typevec_data(emit->buffer);
    size_t length = typevec_len(emit->buffer);
    io_write(io, data, length - 1);
}
