#include <stdlib.h>
#include <stdio.h>

#include "cthulhu/cthulhu.h"

static int next(void *ptr) { return fgetc(ptr); }

static void *alloc(void *self, CtSize size)
{
    (void)self;
    return malloc(size);
}

static void dealloc(void *self, void *ptr)
{
    (void)self;
    free(ptr);
}

int main(void)
{
    CtState state = ctAllocState(NULL, alloc, dealloc);

    CtStream source = ctAllocStream(&state, stdin, next);
    CtParser parser = ctAllocParser(&state, &source);



    ctFreeState(&state);

    return 0;
}
