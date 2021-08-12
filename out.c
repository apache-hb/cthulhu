#ifndef ctu_main
#define ctu_main

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>




int main(void);

int main(void)
{
block0:(void)0;
int local1[10];
int* local2 = local1 + 0;
*local2 = 25;
int* local4 = local1 + 1;
*local4 = 0;
int* local6 = local1 + 2;
*local6 = 50;
int local8[1];
*local8 = 2;
int* local10 = *local1;
int local11 = *local8;
int* local12 = local10 + local11;
*local12 = 0;
int* local14 = *local1;
int* local15 = local14 + 3;
*local15 = 25;
int* local17 = *local1;
int* local18 = local17 + 3;
int local19 = *local18;
return local19;
}

#endif /* ctu_main */
