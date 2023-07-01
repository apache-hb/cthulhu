#include "common.h"

#include "std/vector.h"
#include "std/str.h"

const char *sign_name(sign_t sign)
{
#define SIGN_KIND(ID, STR) case ID: return STR;
    switch (sign)
    {
#include "cthulhu/hlir/hlir.inc"
    default: return "unknown";
    }
}

const char *digit_name(digit_t digit)
{
#define DIGIT_KIND(ID, STR) case ID: return STR;
    switch (digit)
    {
#include "cthulhu/hlir/hlir.inc"
    default: return "unknown";
    }
}

const char *quals_name(quals_t quals)
{
#define TYPE_QUALIFIER(ID, STR, BIT) if (quals & (BIT)) { vector_push(&names, (char*)(STR)); }
    vector_t *names = vector_new(4);
#include "cthulhu/hlir/hlir.inc"
    return str_join(" | ", names);
}
