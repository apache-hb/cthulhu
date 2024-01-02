#include "std/fifo.h"
#include "base/panic.h"
#include "memory/arena.h"
#include <string.h>

typedef struct fifo_t
{
    arena_t *arena;

    size_t typesize;
    size_t size;
    size_t head;
    size_t tail;

    void *data;
} fifo_t;

static void fifo_ensure(fifo_t *fifo, size_t size)
{
    if (fifo->head + size > fifo->size)
    {
        size_t new_size = fifo->size * 2;
        void *new_data = ARENA_MALLOC(fifo->arena, fifo->typesize * new_size, "fifo_data", fifo);

        memcpy(new_data, fifo->data, fifo->typesize * fifo->size);

        fifo->data = new_data;
        fifo->size = new_size;
    }
}

static void *fifo_get(fifo_t *fifo, size_t index)
{
    return (char*)fifo->data + index * fifo->typesize;
}

fifo_t *fifo_new(size_t typesize, size_t size, arena_t *arena)
{
    CTASSERT(typesize > 0);
    CTASSERT(size > 0);
    CTASSERT(arena != NULL);

    fifo_t *fifo = ARENA_MALLOC(arena, sizeof(fifo_t), "fifo_t", NULL);

    fifo->arena = arena;

    fifo->typesize = typesize;
    fifo->size = size;
    fifo->head = 0;
    fifo->tail = 0;

    fifo->data = ARENA_MALLOC(arena, typesize * size, "fifo_data", fifo);

    return fifo;
}

// push an element onto the fifo
void fifo_insert(fifo_t *fifo, const void *data)
{
    CTASSERT(fifo != NULL);
    CTASSERT(data != NULL);

    fifo_ensure(fifo, 1);

    void *dst = fifo_get(fifo, fifo->head);

    memcpy(dst, data, fifo->typesize);

    fifo->head = (fifo->head + 1) % fifo->size;
}

// take the next element off the fifo
void fifo_remove(fifo_t *fifo, void *data)
{
    CTASSERT(fifo != NULL);
    CTASSERT(data != NULL);

    void *src = fifo_get(fifo, fifo->tail);
    memcpy(data, src, fifo->typesize);

    fifo->tail = (fifo->tail + 1) % fifo->size;
}

bool fifo_is_empty(fifo_t *fifo)
{
    CTASSERT(fifo != NULL);

    return fifo->head == fifo->tail;
}
