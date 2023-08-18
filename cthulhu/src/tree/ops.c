#include "common.h"

#include "std/vector.h"
#include "std/str.h"

const char *unary_name(unary_t op)
{
#define UNARY_OP(ID, STR, SYM) case ID: return STR;
    switch (op)
    {
#include "cthulhu/tree/hlir.inc"
    default: return "unknown";
    }
}

const char *binary_name(binary_t op)
{
#define BINARY_OP(ID, STR, SYM) case ID: return STR;
    switch (op)
    {
#include "cthulhu/tree/hlir.inc"
    default: return "unknown";
    }
}

const char *compare_name(compare_t op)
{
#define COMPARE_OP(ID, STR, SYM) case ID: return STR;
    switch (op)
    {
#include "cthulhu/tree/hlir.inc"
    default: return "unknown";
    }
}

const char *unary_symbol(unary_t op)
{
#define UNARY_OP(ID, STR, SYM) case ID: return SYM;
    switch (op)
    {
#include "cthulhu/tree/hlir.inc"
    default: return "unknown";
    }
}

const char *binary_symbol(binary_t op)
{
#define BINARY_OP(ID, STR, SYM) case ID: return SYM;
    switch (op)
    {
#include "cthulhu/tree/hlir.inc"
    default: return "unknown";
    }
}

const char *compare_symbol(compare_t op)
{
#define COMPARE_OP(ID, STR, SYM) case ID: return SYM;
    switch (op)
    {
#include "cthulhu/tree/hlir.inc"
    default: return "unknown";
    }
}

const char *sign_name(sign_t sign)
{
#define SIGN_KIND(ID, STR) case ID: return STR;
    switch (sign)
    {
#include "cthulhu/tree/hlir.inc"
    default: return "unknown";
    }
}

const char *digit_name(digit_t digit)
{
#define DIGIT_KIND(ID, STR) case ID: return STR;
    switch (digit)
    {
#include "cthulhu/tree/hlir.inc"
    default: return "unknown";
    }
}

const char *quals_name(quals_t quals)
{
#define TYPE_QUALIFIER(ID, STR, BIT) if (quals & (BIT)) { vector_push(&names, (char*)(STR)); }
    vector_t *names = vector_new(4);
#include "cthulhu/tree/hlir.inc"
    return str_join(" | ", names);
}

const char *link_name(tree_link_t link)
{
#define HLIR_LINKAGE(ID, STR) case ID: return STR;
    switch (link)
    {
#include "cthulhu/tree/hlir.inc"
    default: return "unknown";
    }
}

const char *vis_name(tree_visible_t vis)
{
#define HLIR_VISIBILITY(ID, STR) case ID: return STR;
    switch (vis)
    {
#include "cthulhu/tree/hlir.inc"
    default: return "unknown";
    }
}