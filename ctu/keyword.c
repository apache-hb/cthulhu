#include "keywords.h"

#define KEY(id, str) case id: return str;
#define OP(id, str) case id: return str;
#define RES(id, str) case id: return str;

const char* ctu_keyword_str(ctu_keyword key)
{
    switch(key)
    {
#include "keywords.inc"
    default: return "invalid";
    }
}