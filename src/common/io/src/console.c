// SPDX-License-Identifier: LGPL-3.0-only

#include "io/console.h"
#include "io/impl.h"

#include "core/macros.h"

#include <stdio.h>

static size_t cout_write(io_t *self, const void *src, size_t size)
{
    CT_UNUSED(self);
    return fwrite(src, 1, size, stdout);
}

static size_t cout_fwrite(io_t *self, const char *fmt, va_list args)
{
    CT_UNUSED(self);
    return vfprintf(stdout, fmt, args);
}

static size_t cerr_write(io_t *self, const void *src, size_t size)
{
    CT_UNUSED(self);
    return fwrite(src, 1, size, stderr);
}

static size_t cerr_fwrite(io_t *self, const char *fmt, va_list args)
{
    CT_UNUSED(self);
    return vfprintf(stderr, fmt, args);
}

// TODO: find a way to simplify this down to a single io_t

static const io_callbacks_t kConsoleOutCallbacks = {
    .fn_write = cout_write,
    .fn_fwrite = cout_fwrite,
};

static const io_callbacks_t kConsoleErrorCallbacks = {
    .fn_write = cerr_write,
    .fn_fwrite = cerr_fwrite,
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
