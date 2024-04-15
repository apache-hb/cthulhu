// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

typedef struct io_t io_t;

typedef struct meta_emit_t {
    io_t *io;

    int indent;
    int depth;
} meta_emit_t;

void emit_init(meta_emit_t *emit, io_t *io, int indent);
meta_emit_t emit_make(io_t *io, int indent);

void emit_comment(meta_emit_t *emit, const char *comment);

void emit_printf(meta_emit_t *emit, const char *fmt, ...);
void emit_print(meta_emit_t *emit, const char *line);

void emit_indent(meta_emit_t *emit);
void emit_unindent(meta_emit_t *emit);
