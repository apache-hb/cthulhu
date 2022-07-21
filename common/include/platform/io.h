#pragma once

#include <stddef.h>

typedef size_t(*file_read_t)(void *self, void *dst, size_t size);
typedef size_t(*file_write_t)(void *self, void *src, size_t size);

typedef struct file_t {
    void *data;
} file_t;
