#ifndef ctu_main
#define ctu_main

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>



const int main(void);
static const int fac(const int);

const int main(void)
{
const int(*local1)(const int) = *fac;
const int local2 = local1(5);
return local2;
}

static const int fac(const int arg0)
{
int local5[1];
local5[0] = 1;
block7:(void)0;
int local8 = *local5;
bool local10 = local8 <= arg0;
if (local10) { goto block12; } else { goto block23; }
block12:(void)0;
int local19 = *local5;
const int local20 = local19 + 1;
local5[0] = local20;
goto block7;
}

#endif /* ctu_main */
