#ifndef ctu_main
#define ctu_main

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

int main();
static int fac(int);

int main()
{
int local1 = fac(5);
return local1;
}

static int fac(int arg0)
{
int local1[1];
*local1 = 1;
int local3[1];
*local3 = 1;
block5:(void)0;
int local6 = *local3;
bool local7 = local6 <= arg0;
if (local7) { goto block9; } else { goto block19; }
block9:(void)0;
int local11 = *local1;
int local12 = *local3;
int local13 = local11 * local12;
*local1 = local13;
int local15 = *local3;
int local16 = local15 + 1;
*local3 = local16;
goto block5;
block19:(void)0;
int local20 = *local1;
return local20;
}

#endif /* ctu_main */
