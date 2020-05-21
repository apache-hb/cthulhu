#include "keywords.h"

const char* keyword_to_string(keyword key)
{
    switch(key)
    {
#define KEY(id, str) case id: return str;
#define OP(id, str) case id: return str;
#define ASM(id, str) case id: return str;
#include "keywords.inc"
    default: return "invalid";
    }
}