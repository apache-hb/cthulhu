#ifndef ctu_main
#define ctu_main

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>



const int main(int, const char**);
static const int fib(int);

const int main(int arg0, const char** arg1)
{
const int(*local5)(const int) = *fib;
const int local6 = local5(40);
return local6;
}

static const int fib(int arg0)
{
bool local5 = arg0 < 2;
if (local5) { goto block7; } else { goto block10; }
block7:(void)0;
return arg0;
block10:(void)0;
const int(*local11)(const int) = *fib;
const int local13 = arg0 - 1;
const int local14 = local11(local13);
const int(*local15)(const int) = *fib;
const int local17 = arg0 - 2;
const int local18 = local15(local17);
const int local19 = local14 + local18;
return local19;
}

#endif /* ctu_main */
