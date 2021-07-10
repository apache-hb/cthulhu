#pragma once

#include <stddef.h>

#define MAX(L, R) ((L) > (R) ? (L) : (R)) 
#define MIN(L, R) ((L) < (R) ? (L) : (R)) 

void *ctu_malloc(size_t size);
void *ctu_realloc(void *ptr, size_t size);
void ctu_free(void *ptr);
