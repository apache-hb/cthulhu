#ifndef ctu_main
#define ctu_main

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

union type_17_t;
union type_17_t { int _0;bool _1;};

const union type_17_t test(void);

const union type_17_t test(void)
{
block0:(void)0;
union type_17_t local1[1];
local1->_0 = 20;
local1->_1 = true;
union type_17_t local4 = *local1;
return local4;
}

#endif /* ctu_main */
