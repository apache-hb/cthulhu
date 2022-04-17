#pragma once

#include <stddef.h>

/**
 * a map size.
 * 
 * a curious case with compilers is that maps either need to be massive
 * or miniscule. a toplevel module may have hundreds of functions, variables, imports etc.
 * then function bodies maybe have 10 variables including arguments.
 * 
 * symbol tables seemingly follow this rule of either exporting 1 or 2 functions
 * or exporting an entire library with thousands of symbols.
 *
 * we pick a few arbitrary primes
 */

#define MAP_SMALL 7 /// optimal for maps with less than 10 items
#define MAP_BIG 97 /// optimal for maps with 100 or more items
#define MAP_MASSIVE 1009 /// optimal for maps with 1000 or more items
