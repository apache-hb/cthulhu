// SPDX-License-Identifier: LGPL-3.0-or-later
#include "core/macros.h"
#include "meta/meta.h"
#include "common.h"

#include "io/io.h"
#include "base/panic.h"

void emit_init(meta_emit_t *emit, io_t *io, unsigned indent)
{
    emit->io = io;
    emit->indent = indent;
    emit->depth = 0;
}

meta_emit_t emit_make(io_t *io, unsigned indent)
{
    meta_emit_t emit;
    emit_init(&emit, io, indent);
    return emit;
}

void emit_comment(meta_emit_t *emit, const char *comment)
{
    emit_indent(emit);
    emit_printf(emit, "/* %s */\n", comment);
    emit_unindent(emit);
}

static void emit_line_indent(meta_emit_t *emit)
{
    for (unsigned i = 0; i < emit->depth; i++) {
        io_printf(emit->io, " ");
    }
}

void emit_printf(meta_emit_t *emit, const char *fmt, ...)
{
    emit_line_indent(emit);

    va_list args;
    va_start(args, fmt);

    io_vprintf(emit->io, fmt, args);
    io_printf(emit->io, "\n");

    va_end(args);
}

void emit_print(meta_emit_t *emit, const char *line)
{
    emit_line_indent(emit);

    io_printf(emit->io, "%s\n", line);
}

void emit_indent(meta_emit_t *emit)
{
    emit->depth += emit->indent;
}

void emit_unindent(meta_emit_t *emit)
{
    emit->depth -= emit->indent;
}

void meta_emit_common(io_t *header, io_t *source)
{
    CTASSERT(header != NULL);
    CTASSERT(source != NULL);

    meta_emit_t h = emit_make(header, 4);
    meta_emit_t s = emit_make(source, 4);

    emit_print(&h, "// SPDX-License-Identifier: LGPL-3.0-or-later");
    emit_print(&s, "// SPDX-License-Identifier: LGPL-3.0-or-later");

    emit_print(&h, "// This file is generated. Do not edit.");
    emit_print(&s, "// This file is generated. Do not edit.");
}
