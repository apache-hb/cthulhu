// SPDX-License-Identifier: LGPL-3.0-or-later
#include "cthulhu/cinterop/writer.h"

#include "io/io.h"

#include "base/panic.h"
#include "base/util.h"

static void emit_indent(ctu_writer_t *writer)
{
    CTASSERT(writer != NULL);

    for (int i = 0; i < writer->depth * writer->indent; i++)
    {
        io_write(writer->io, " ", 1);
    }
}

void writer_init(ctu_writer_t *writer, io_t *io, int indent)
{
    CTASSERT(writer != NULL);
    CTASSERT(io != NULL);

    ctu_writer_t self = {
        .io = io,
        .indent = indent,
        .depth = 0
    };

    *writer = self;
}

void writer_print(ctu_writer_t *writer, const char *str)
{
    CTASSERT(writer != NULL);
    emit_indent(writer);
    io_write(writer->io, str, ctu_strlen(str));
}

void writer_printf(ctu_writer_t *writer, const char *fmt, ...)
{
    CTASSERT(writer != NULL);
    CTASSERT(fmt != NULL);

    va_list args;
    va_start(args, fmt);

    emit_indent(writer);
    io_vprintf(writer->io, fmt, args);

    va_end(args);
}

void writer_println(ctu_writer_t *writer, const char *str)
{
    CTASSERT(writer != NULL);
    emit_indent(writer);
    io_write(writer->io, str, ctu_strlen(str));
    writer_newline(writer);
}

void writer_printfln(ctu_writer_t *writer, const char *fmt, ...)
{
    CTASSERT(writer != NULL);
    CTASSERT(fmt != NULL);

    va_list args;
    va_start(args, fmt);

    emit_indent(writer);
    io_vprintf(writer->io, fmt, args);
    writer_newline(writer);

    va_end(args);
}

void writer_indent(ctu_writer_t *writer)
{
    CTASSERT(writer != NULL);
    writer->depth++;
}

void writer_dedent(ctu_writer_t *writer)
{
    CTASSERT(writer != NULL);
    CTASSERTF(writer->depth > 0, "cannot dedent past 0");

    writer->depth--;
}

void writer_newline(ctu_writer_t *writer)
{
    CTASSERT(writer != NULL);

    io_write(writer->io, "\n", 1);
}
