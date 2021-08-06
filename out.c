#ifndef ctu_main
#define ctu_main

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>



static int global0[1];

const int main(void);
static const int f(int);

const int main(void)
{
block0:(void)0;
const int local1 = 1 % 0;
*global0 = local1;
const int local3 = f(200);
return local3;
}

static const int f(int arg0)
{
int local0[1];
*local0 = arg0;
block2:(void)0;
int local3 = *global0;
int local4 = *local0;
const int local5 = local3 + local4;
return local5;
}

#endif /* ctu_m