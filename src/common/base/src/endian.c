#include "base/endian.h"
#include "base/panic.h"

#include <stdlib.h> // IWYU pragma: keep

uint16_t endian_swap16(uint16_t value)
{
    return BYTESWAP_U16(value);
}

uint32_t endian_swap32(uint32_t value)
{
    return BYTESWAP_U32(value);
}

uint64_t endian_swap64(uint64_t value)
{
    return BYTESWAP_U64(value);
}

USE_DECL
uint16_t native_order16(uint16_t value, endian_t order)
{
    CTASSERTF(order < eEndianTotal, "invalid endian order: %d", order);
    return (order == eEndianNative) ? value : endian_swap16(value);
}

USE_DECL
uint32_t native_order32(uint32_t value, endian_t order)
{
    CTASSERTF(order < eEndianTotal, "invalid endian order: %d", order);
    return (order == eEndianNative) ? value : endian_swap32(value);
}

USE_DECL
uint64_t native_order64(uint64_t value, endian_t order)
{
    CTASSERTF(order < eEndianTotal, "invalid endian order: %d", order);
    return (order == eEndianNative) ? value : endian_swap64(value);
}
