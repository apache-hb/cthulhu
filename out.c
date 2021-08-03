#ifndef ctu_main
#define ctu_main

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

struct type_17_t;
struct type_17_t { const int _0;const bool _1;};

static const size_t closure0(void);
static const size_t size2(void);
const int main(int, const const char**);

static const size_t closure0(void)
{
const size_t local0 = sizeof(const struct type_17_t);
return local0;
}

static const size_t size2(void)
{
const size_t local1 = closure0();
const size_t local2 = local1 * 2;
return local2;
}

const int main(int arg0, const const char** arg1)
{
const size_t local5 = closure0();
const int local6 = (const int)local5;
return local6;
}

#endif /* ctu_main */
