#ifndef ctu_main
#define ctu_main

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>




const int main(void);

const int main(void)
{
int local1[3][3];
const int local2 = local1 + 0;
*local2 = 25;
const int local4 = local1 + 1;
*local4 = 0;
const int local6 = local1 + 2;
*local6 = 50;
int* local14 = local1 + 2;
*local14 = 0;
int* local17 = local1 + 3;
*local17 = 25;
int* local20 = local1 + 3;
int local21 = *local20;
return local21;
}

#endif /* ctu_main */
