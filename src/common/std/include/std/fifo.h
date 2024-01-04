#pragma once

#include <stdbool.h>

typedef struct arena_t arena_t;

typedef struct fifo_t fifo_t;

// a fifo ring buffer
fifo_t *fifo_new(size_t typesize, size_t size, arena_t *arena);

// push an element onto the fifo
void fifo_insert(fifo_t *fifo, const void *data);

// take the next element off the fifo
void fifo_remove(fifo_t *fifo, void *data);

// check if the fifo is empty
bool fifo_is_empty(fifo_t *fifo);
