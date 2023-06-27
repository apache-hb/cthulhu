#pragma once

typedef enum 
{
#define HLIR_ARITY(ID, STR) ID,
#include "hlir-def.inc"
    eArityTotal
} arity_t;

const char *arity_to_string(arity_t self);
