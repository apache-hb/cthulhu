#ifndef ctu_main
#define ctu_main

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
const char *str0 = "\x68\x65\x6c\x6c\x6f";


static const char* hello(void);
const int main(const int, const char**);

static const char* hello(void)
{
return str0;
}

const int main(const int arg0, const char** arg1)
{
const char*(*local5)() = *hello;
const char* local6 = local5();
}

#endif /* ctu_main */
