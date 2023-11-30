#pragma once

#include "base/compiler.h"

#if OS_WINDOWS
#   include "base/win32.h"
#   define CTU_BIG_ENDIAN REG_DWORD_BIG_ENDIAN
#   define CTU_LITTLE_ENDIAN REG_DWORD_LITTLE_ENDIAN
#   define CTU_BYTE_ORDER REG_DWORD
#else
#   if OS_LINUX
#       include <endian.h>
#   elif OS_MACOS
#       include <machine/endian.h>
#   endif
#   define CTU_BIG_ENDIAN __ORDER_BIG_ENDIAN__
#   define CTU_LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#   define CTU_BYTE_ORDER __BYTE_ORDER__
#endif

#include <stdint.h>

BEGIN_API

///
/// @defgroup Endian Endianess
/// @brief Endianess and byte swapping
/// @{
///

/// @brief endianess enum
typedef enum endian_t
{
#define ENDIAN(id, name, v) id = (v),
#include "endian.inc"

    eEndianTotal
} endian_t;

/// @brief swap the endianess of a 16-bit value
///
/// @param value the value to swap
///
/// @return the swapped value
uint16_t endian_swap16(uint16_t value);

/// @brief swap the endianess of a 32-bit value
///
/// @param value the value to swap
///
/// @return the swapped value
uint32_t endian_swap32(uint32_t value);

/// @brief swap the endianess of a 64-bit value
///
/// @param value the value to swap
///
/// @return the swapped value
uint64_t endian_swap64(uint64_t value);

/// @brief convert a 16-bit value of a given endianess to the native endianess
///
/// @param value the value to convert
/// @param order the endianess of @a value
///
/// @return the converted value
uint16_t native_order16(uint16_t value, endian_t order);

/// @brief convert a 32-bit value of a given endianess to the native endianess
///
/// @param value the value to convert
/// @param order the endianess of @a value
///
/// @return the converted value
uint32_t native_order32(uint32_t value, endian_t order);

/// @brief convert a 64-bit value of a given endianess to the native endianess
///
/// @param value the value to convert
/// @param order the endianess of @a value
///
/// @return the converted value
uint64_t native_order64(uint64_t value, endian_t order);

/// @}

END_API
