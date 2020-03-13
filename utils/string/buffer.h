#ifndef BUFFER_H
#define BUFFER_H

#include <string.h>

typedef struct {
    char* data;
    size_t size;
    size_t allocated;
} buffer_t;

buffer_t* buffer_new();
void buffer_push(buffer_t* self, char c);
buffer_t* buffer_slice(buffer_t* self, size_t begin, size_t end);

#endif /* BUFFER_H */
