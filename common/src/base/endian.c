#include "base/endian.h"

uint16_t endian_swap16(uint16_t value)
{
#if CC_MSVC
    return _byteswap_ushort(value);
#else
    return __builtin_bswap16(value);
#endif
}

uint32_t endian_swap32(uint32_t value)
{
#if CC_MSVC
    return _byteswap_ulong(value);
#else
    return __builtin_bswap32(value);
#endif
}

uint64_t endian_swap64(uint64_t value)
{
#if CC_MSVC
    return _byteswap_uint64(value);
#else
    return __builtin_bswap64(value);
#endif
}

uint16_t native_order16(uint16_t value, endian_t order)
{
    return (order == eEndianNative) ? value : endian_swap16(value);
}

uint32_t native_order32(uint32_t value, endian_t order)
{
    return (order == eEndianNative) ? value : endian_swap32(value);
}

uint64_t native_order64(uint64_t value, endian_t order)
{
    return (order == eEndianNative) ? value : endian_swap64(value);
}
