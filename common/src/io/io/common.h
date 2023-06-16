#pragma once

#include "io/io.h"

typedef size_t (*io_read_t)(io_t *self, void *dst, size_t size);
typedef size_t (*io_write_t)(io_t *self, const void *src, size_t size);

typedef size_t (*io_size_t)(io_t *self);

typedef const void *(*io_map_t)(io_t *self);

typedef void (*io_close_t)(io_t *self);

typedef struct io_callbacks_t
{
    io_read_t fnRead;
    io_write_t fnWrite;

    io_size_t fnGetSize;

    io_map_t fnMap;

    io_close_t fnClose;
} io_callbacks_t;

typedef struct io_t
{
    const io_callbacks_t *cb;

    io_error_t error;
    file_flags_t flags;
    const char *name;
    char data[];
} io_t;
