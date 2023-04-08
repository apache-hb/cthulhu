#include "cthulhu/hlir/arity.h"

const char *arity_to_string(arity_t self)
{
    switch (self)
    {
#define HLIR_ARITY(ID, STR) case ID: return STR;
#include "cthulhu/hlir/hlir-def.inc"
    default: return "unknown";
    }
}
