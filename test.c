#ifndef ctu_main
#define ctu_main

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

static const int take0(const int);
static const int take1(const int*);
static const int take3(const int***);
static const int test();

static const int take0(const int arg0)
{
block0:
return arg0;
}

static const int take1(const int* arg0)
{
block0:
const int local1 = *arg0;
return local1;
}

static const int take3(const int*** arg0)
{
block0:
const int** local1 = *arg0;
const int* local2 = *local1;
const int local3 = *local2;
return local3;
}

static const int test()
{
block0:
int local1[1];
*local1 = 20;
int local3[1];
*local3 = local1;
const int local5 = take1(local3);
int local6[1];
*local6 = local5;
int local8[1];
*local8 = 0;
int local10[1];
*local10 = local8;
const int* local12[1];
*local12 = local10;
const int* local14[1];
*local14 = local12;
const int** local16[1];
*local16 = local14;
const int** local18[1];
*local18 = local16;
const int*** local20[1];
*local20 = local18;
const int*** local22 = *local20;
const int local23 = take3(local22);
int local24[1];
*local24 = 0;
int local26[1];
*local26 = local24;
const int* local28[1];
*local28 = local26;
const int* local30[1];
*local30 = local28;
const int** local32[1];
*local32 = local30;
const int** local34[1];
*local34 = local32;
const int*** local36[1];
*local36 = local34;
const int*** local38 = *local36;
const int** local39 = *local38;
const int* local40 = *local39;
const int local41 = *local40;
const int local42 = take0(local41);
const int local43 = *local24;
return local43;
}

#endif /* ctu_main */
