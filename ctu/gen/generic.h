#pragma once

#include <stddef.h>
#include <stdint.h>

typedef uint8_t byte_t;

typedef struct {
    byte_t *code;
    size_t len, size;
} blob_t;
