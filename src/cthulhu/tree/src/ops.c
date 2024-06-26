// SPDX-License-Identifier: LGPL-3.0-only

#include "cthulhu/tree/ops.h"

#include "arena/arena.h"
#include "base/panic.h"
#include "memory/memory.h"
#include "std/vector.h"
#include "std/str.h"
#include "base/util.h"

static const char *const kUnaryNames[eUnaryTotal] = {
#define UNARY_OP(ID, STR, SYM) [ID] = (STR),
#include "cthulhu/tree/tree.inc"
};

STA_DECL
const char *unary_name(unary_t op)
{
    CTASSERTF(op < eUnaryTotal, "invalid unary operator: %d", op);
    return kUnaryNames[op];
}

static const char *const kUnarySymbols[eUnaryTotal] = {
#define UNARY_OP(ID, STR, SYM) [ID] = (SYM),
#include "cthulhu/tree/tree.inc"
};

STA_DECL
const char *unary_symbol(unary_t op)
{
    CTASSERTF(op < eUnaryTotal, "invalid unary operator: %d", op);
    return kUnarySymbols[op];
}

static const char *const kBinaryNames[eBinaryTotal] = {
#define BINARY_OP(ID, STR, SYM) [ID] = (STR),
#include "cthulhu/tree/tree.inc"
};

STA_DECL
const char *binary_name(binary_t op)
{
    CTASSERTF(op < eBinaryTotal, "invalid binary operator: %d", op);
    return kBinaryNames[op];
}

static const char *const kBinarySymbols[eBinaryTotal] = {
#define BINARY_OP(ID, STR, SYM) [ID] = (SYM),
#include "cthulhu/tree/tree.inc"
};

STA_DECL
const char *binary_symbol(binary_t op)
{
    CTASSERTF(op < eBinaryTotal, "invalid binary operator: %d", op);
    return kBinarySymbols[op];
}

static const char *const kCompareNames[eCompareTotal] = {
#define COMPARE_OP(ID, STR, SYM) [ID] = (STR),
#include "cthulhu/tree/tree.inc"
};

STA_DECL
const char *compare_name(compare_t op)
{
    CTASSERTF(op < eCompareTotal, "invalid compare operator: %d", op);
    return kCompareNames[op];
}

static const char *const kCompareSymbols[eCompareTotal] = {
#define COMPARE_OP(ID, STR, SYM) [ID] = (SYM),
#include "cthulhu/tree/tree.inc"
};

STA_DECL
const char *compare_symbol(compare_t op)
{
    CTASSERTF(op < eCompareTotal, "invalid compare operator: %d", op);
    return kCompareSymbols[op];
}

static const char *const kSignNames[eSignTotal] = {
#define SIGN_KIND(ID, STR) [ID] = (STR),
#include "cthulhu/tree/tree.inc"
};

STA_DECL
const char *sign_name(sign_t sign)
{
    CTASSERTF(sign < eSignTotal, "invalid sign: %d", sign);
    return kSignNames[sign];
}

static const char *const kDigitNames[eDigitTotal] = {
#define DIGIT_KIND(ID, STR) [ID] = (STR),
#include "cthulhu/tree/tree.inc"
};

STA_DECL
const char *digit_name(digit_t digit)
{
    CTASSERTF(digit < eDigitTotal, "invalid digit: %d", digit);
    return kDigitNames[digit];
}

STA_DECL
const char *quals_string(tree_quals_t quals)
{
    arena_t *arena = get_global_arena();

    // unhinged macro abuse to make the array size known at compile time
    char buffer[
        1
#define TYPE_QUALIFIER(ID, STR, BIT) + sizeof(STR)
#include "cthulhu/tree/tree.inc"
    ];

    char *ptr = buffer;

#define TYPE_QUALIFIER(ID, STR, BIT) \
    if (quals & (ID)) { \
        if (ptr != buffer) \
            *ptr++ = '|'; \
        ctu_memcpy(ptr, STR, sizeof(STR)); \
        ptr += sizeof(STR) - 1; \
    }
#include "cthulhu/tree/tree.inc"

    *ptr = '\0';
    return arena_strndup(buffer, ptr - buffer, arena);
}

static const char *const kLinkNames[eLinkTotal] = {
#define TREE_LINKAGE(ID, STR) [ID] = (STR),
#include "cthulhu/tree/tree.inc"
};

STA_DECL
const char *linkage_string(tree_linkage_t link)
{
    CTASSERTF(link < eLinkTotal, "invalid link: %d", link);
    return kLinkNames[link];
}

static const char *const kVisibilityNames[eVisibileTotal] = {
#define TREE_VISIBILITY(ID, STR) [ID] = (STR),
#include "cthulhu/tree/tree.inc"
};

STA_DECL
const char *visibility_string(tree_visibility_t vis)
{
    CTASSERTF(vis < eVisibileTotal, "invalid visibility: %d", vis);
    return kVisibilityNames[vis];
}
