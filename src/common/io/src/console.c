// SPDX-License-Identifier: LGPL-3.0-only

#include "io/console.h"
#include "io/impl.h"

#include "core/macros.h"

#include <stdio.h>

static size_t con_out_write(io_t *self, const char *fmt, va_list args)
{
    CT_UNUSED(self);

    return vfprintf(stdout, fmt, args);
}

static size_t con_error_write(io_t *self, const char *fmt, va_list args)
{
    CT_UNUSED(self);

    return vfprintf(stderr, fmt, args);
}

// TODO: find a way to simplify this down to a single io_t

static const io_callbacks_t kConsoleOutCallbacks = {
    .fn_write_format = con_out_write,
};

static const io_callbacks_t kConsoleErrorCallbacks = {
    .fn_write_format = con_error_write,
};

static io_t gConsoleOutIo = {
    .cb = &kConsoleOutCallbacks,
    .flags = eOsAccessWrite,
    .arena = NULL,
    .name = "stdout",
};

static io_t gConsoleErrorIo = {
    .cb = &kConsoleErrorCallbacks,
    .flags = eOsAccessWrite,
    .arena = NULL,
    .name = "stderr",
};

io_t *io_stdout(void)
{
    return &gConsoleOutIo;
}

io_t *io_stderr(void)
{
    return &gConsoleErrorIo;
}
