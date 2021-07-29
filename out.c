#ifndef ctu_main
#define ctu_main

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <stdio.h>
const char *str0 = "\x68\x65\x6c\x6c\x6f\x20\x77\x6f\x72\x6c\x64";


const int main(int, const char**);

const int main(int arg0, const char** arg1)
{
int local0[1];
local0[0] = arg0;
const char** local2[1];
local2[0] = arg1;
block4:(void)0;
const int(*local5)(const char*) = *puts;
const int local6 = local5(str0);
return 0;
}

#endif /* ctu_main */
