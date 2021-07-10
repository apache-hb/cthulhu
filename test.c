#ifndef ctu_main
#define ctu_main

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

static const int take0(const int);
static const int take1(const int*);
static const int take3(const int***);
static const int refarg(const int);
static const int test();

static const int take0(const int arg0)
{
int local0[1];
*local0 = arg0;
block2:(void)0;
const int local3 = *local0;
return local3;
}

static const int take1(const int* arg0)
{
const int* local0[1];
*local0 = arg0;
block2:(void)0;
const int* local3 = *local0;
const int local4 = *local3;
return local4;
}

static const int take3(const int*** arg0)
{
const int*** local0[1];
*local0 = arg0;
block2:(void)0;
const int*** local3 = *local0;
const int** local4 = *local3;
const int* local5 = *local4;
const int local6 = *local5;
return local6;
}

static const int refarg(const int arg0)
{
int local0[1];
*local0 = arg0;
block2:(void)0;
const int(*local3)(const int*) = *take1;
const int local4 = local3(local0);
return local4;
}

static const int test()
{
block0:(void)0;
int local1[1];
*local1 = 20;
const int(*local3)(const int*) = *take1;
const int local4 = local3(local1);
int local5[1];
*local5 = local4;
int local7[1];
*local7 = 0;
const int* local9[1];
*local9 = local7;
const int** local11[1];
*local11 = local9;
const int*** local13[1];
*local13 = local11;
const int(*local15)(const int***) = *take3;
const int*** local16 = *local13;
const int local17 = local15(local16);
int local18[1];
*local18 = local17;
int local20[1];
*local20 = 0;
const int* local22[1];
*local22 = local20;
const int** local24[1];
*local24 = local22;
const int*** local26[1];
*local26 = local24;
const int(*local28)(const int) = *take0;
const int*** local29 = *local26;
const int** local30 = *local29;
const int* local31 = *local30;
const int local32 = *local31;
const int local33 = local28(local32);
int local34[1];
*local34 = local33;
const int(*local36)(const int) = *refarg;
const int local37 = *local20;
const int local38 = local36(local37);
return local38;
}

#endif /* ctu_main */
