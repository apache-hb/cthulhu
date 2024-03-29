// SPDX-License-Identifier: LGPL-3.0-only

#include "endian/endian.h"

#include "base/panic.h"

// _byteswap_ushort, _byteswap_ulong, _byteswap_uint64
#if CT_CC_MSVC || CT_CC_CLANGCL
#   include <stdlib.h> // IWYU pragma: keep
#endif

uint16_t endian_swap16(uint16_t value)
{
    return CT_BSWAP_U16(value);
}

uint32_t endian_swap32(uint32_t value)
{
    return CT_BSWAP_U32(value);
}

uint64_t endian_swap64(uint64_t value)
{
    return CT_BSWAP_U64(value);
}

USE_DECL
uint16_t native_order16(uint16_t value, endian_t order)
{
    CTASSERTF(order < eEndianCount, "invalid endian order: %d", order);
    return (order == eEndianNative) ? value : endian_swap16(value);
}

USE_DECL
uint32_t native_order32(uint32_t value, endian_t order)
{
    CTASSERTF(order < eEndianCount, "invalid endian order: %d", order);
    return (order == eEndianNative) ? value : endian_swap32(value);
}

USE_DECL
uint64_t native_order64(uint64_t value, endian_t order)
{
    CTASSERTF(order < eEndianCount, "invalid endian order: %d", order);
    return (order == eEndianNative) ? value : endian_swap64(value);
}
