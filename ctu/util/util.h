#pragma once

#include <stddef.h>

#define MAX(L, R) ((L) > (R) ? (L) : (R)) 

void *copyof(void *ptr, size_t size);
