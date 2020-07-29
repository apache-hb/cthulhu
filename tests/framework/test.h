#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdlib.h>

#define CT_MALLOC malloc
#define CT_REALLOC realloc
#define CT_FREE free

#include "cthulhu/cthulhu.c"

typedef struct {
    const char *str;
    size_t idx;
    size_t len;
} TestStream;

TestStream testStream(const char *text)
{
    TestStream stream;

    stream.str = text;
    stream.idx = 0;
    stream.len = strlen(text);

    return stream;
}

static int testNext(void *ptr)
{
    TestStream *stream = (TestStream*)ptr;
    
    return (stream.idx < stream.len) ? stream.str[stream.idx++] : -1;
}

#define MAKE_TEST(name, ...) int main(int argc, const char **argv) \
{   \
    printf("Running test " name "\n"); \
    { __VA_ARGS__ } \
}

#define LEXING_STATE(state, stream)

#endif /* TEST_FRAMEWORK_H */
