#include "io/console.h"
#include "core/macros.h"
#include "io/impl.h"
#include <stdio.h>

typedef struct io_console_t
{
    // empty for now
    int dummy;
} io_console_t;

static size_t con_write(io_t *self, const void *src, size_t size)
{
    CTU_UNUSED(self);

    return fwrite(src, 1, size, stdout);
}

static const io_callbacks_t kConsoleCallbacks = {
    .fn_read = NULL,
    .fn_write = con_write,

    .fn_get_size = NULL,
    .fn_seek = NULL,

    .fn_map = NULL,
    .fn_close = NULL
};

io_t *io_stdout(void)
{
    io_console_t console = { 0 };

    return io_new(&kConsoleCallbacks, eAccessWrite, "stdout", &console, sizeof(io_console_t));
}
