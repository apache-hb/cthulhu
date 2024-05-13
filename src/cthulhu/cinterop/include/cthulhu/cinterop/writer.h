// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <ctu_cinterop_api.h>

typedef struct io_t io_t;

CT_BEGIN_API

typedef struct ctu_writer_t {
    io_t *io;

    int indent;
    int depth;
} ctu_writer_t;

CT_CINTEROP_API void writer_init(ctu_writer_t *writer, io_t *io, int indent);

CT_CINTEROP_API void writer_print(ctu_writer_t *writer, const char *str);

CT_CINTEROP_API void writer_printf(ctu_writer_t *writer, const char *fmt, ...);

CT_CINTEROP_API void writer_println(ctu_writer_t *writer, const char *str);

CT_CINTEROP_API void writer_printfln(ctu_writer_t *writer, const char *fmt, ...);

CT_CINTEROP_API void writer_indent(ctu_writer_t *writer);

CT_CINTEROP_API void writer_dedent(ctu_writer_t *writer);

CT_CINTEROP_API void writer_newline(ctu_writer_t *writer);

CT_END_API
