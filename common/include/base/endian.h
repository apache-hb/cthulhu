#pragma once

#include "base/compiler.h"

#if OS_WINDOWS
#   include <Windows.h>
#   define BIG_ENDIAN REG_DWORD_BIG_ENDIAN
#   define LITTLE_ENDIAN REG_DWORD_LITTLE_ENDIAN
#   define BYTE_ORDER REG_DWORD
#else
#   if OS_LINUX
#       include <endian.h>
#   elif OS_MACOS
#       include <machine/endian.h>
#   endif
#   define BIG_ENDIAN __ORDER_BIG_ENDIAN__
#   define LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#   define BYTE_ORDER __BYTE_ORDER__
#endif

#include <stdint.h>

/**
 * @defgroup Endian Endianess
 * @brief Endianess and byte swapping
 * @{
 */

typedef enum endian_t 
{
#define ENDIAN(id, name, v) id = (v),
#include "endian.inc"

    eEndianTotal
} endian_t;

uint16_t endian_swap16(uint16_t value);
uint32_t endian_swap32(uint32_t value);
uint64_t endian_swap64(uint64_t value);

uint16_t native_order16(uint16_t value, endian_t order);
uint32_t native_order32(uint32_t value, endian_t order);
uint64_t native_order64(uint64_t value, endian_t order);

/** @} */
