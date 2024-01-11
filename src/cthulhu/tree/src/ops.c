#include "cthulhu/tree/ops.h"

#include "std/vector.h"
#include "std/str.h"

const char *unary_name(unary_t op)
{
#define UNARY_OP(ID, STR, SYM) case ID: return STR;
    switch (op)
    {
#include "cthulhu/tree/tree.inc"
    default: return "unknown";
    }
}

const char *binary_name(binary_t op)
{
#define BINARY_OP(ID, STR, SYM) case ID: return STR;
    switch (op)
    {
#include "cthulhu/tree/tree.inc"
    default: return "unknown";
    }
}

const char *compare_name(compare_t op)
{
#define COMPARE_OP(ID, STR, SYM) case ID: return STR;
    switch (op)
    {
#include "cthulhu/tree/tree.inc"
    default: return "unknown";
    }
}

const char *unary_symbol(unary_t op)
{
#define UNARY_OP(ID, STR, SYM) case ID: return SYM;
    switch (op)
    {
#include "cthulhu/tree/tree.inc"
    default: return "unknown";
    }
}

const char *binary_symbol(binary_t op)
{
#define BINARY_OP(ID, STR, SYM) case ID: return SYM;
    switch (op)
    {
#include "cthulhu/tree/tree.inc"
    default: return "unknown";
    }
}

const char *compare_symbol(compare_t op)
{
#define COMPARE_OP(ID, STR, SYM) case ID: return SYM;
    switch (op)
    {
#include "cthulhu/tree/tree.inc"
    default: return "unknown";
    }
}

const char *sign_name(sign_t sign)
{
#define SIGN_KIND(ID, STR) case ID: return STR;
    switch (sign)
    {
#include "cthulhu/tree/tree.inc"
    default: return "unknown";
    }
}

const char *digit_name(digit_t digit)
{
#define DIGIT_KIND(ID, STR) case ID: return STR;
    switch (digit)
    {
#include "cthulhu/tree/tree.inc"
    default: return "unknown";
    }
}

#define STRING_LENGTH kQualStringLen
#define FLAG_COUNT kQualFlagsCount

static const size_t kQualFlagsCount = 1
#define TYPE_QUALIFIER(ID, STR, BIT) + 1
#include "cthulhu/tree/tree.inc"
;

static const size_t kQualStringLen = sizeof(
#define TYPE_QUALIFIER(ID, STR, BIT) STR
#include "cthulhu/tree/tree.inc"
);

const char *quals_name(quals_t quals)
{
#define TYPE_QUALIFIER(ID, STR, BIT) if (quals & (BIT)) { vector_push(&names, (char*)(STR)); }
    vector_t *names = vector_new(4);
#include "cthulhu/tree/tree.inc"
    return str_join(" | ", names);
}

const char *link_name(tree_link_t link)
{
#define TREE_LINKAGE(ID, STR) case ID: return STR;
    switch (link)
    {
#include "cthulhu/tree/tree.inc"
    default: return "unknown";
    }
}

const char *vis_name(visibility_t vis)
{
#define TREE_VISIBILITY(ID, STR) case ID: return STR;
    switch (vis)
    {
#include "cthulhu/tree/tree.inc"
    default: return "unknown";
    }
}
