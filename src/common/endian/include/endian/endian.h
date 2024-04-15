// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_endian_api.h>

#include "core/analyze.h"
#include "core/compiler.h"

#include <stdint.h>

/// @defgroup endian Endianess and byte swapping
/// @ingroup common
/// @{

#if CT_OS_WINDOWS
#   include "core/win32.h" // IWYU pragma: keep
#   define CT_BIG_ENDIAN REG_DWORD_BIG_ENDIAN
#   define CT_LITTLE_ENDIAN REG_DWORD_LITTLE_ENDIAN
#   define CT_BYTE_ORDER REG_DWORD
#else
#   if CT_OS_LINUX
#      include <endian.h>
#   elif CT_OS_APPLE
#      include <machine/endian.h>
#   endif
#   define CT_BIG_ENDIAN __ORDER_BIG_ENDIAN__
#   define CT_LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#   define CT_BYTE_ORDER __BYTE_ORDER__
#endif

/// @def CT_BIG_ENDIAN
/// @brief the big endian byte order
/// @def CT_LITTLE_ENDIAN
/// @brief the little endian byte order
/// @def CT_BYTE_ORDER
/// @brief the native byte order

CT_BEGIN_API

/// @brief endianess enum
typedef enum endian_t
{
#define ENDIAN(id, name, v) id = (v),
#include "endian.inc"

    eEndianCount
} endian_t;

/// @brief swap the endianess of a 16-bit value
///
/// @param value the value to swap
///
/// @return the swapped value
CT_NODISCARD CT_CONSTFN
CT_ENDIAN_API uint16_t endian_swap16(uint16_t value);

/// @brief swap the endianess of a 32-bit value
///
/// @param value the value to swap
///
/// @return the swapped value
CT_NODISCARD CT_CONSTFN
CT_ENDIAN_API uint32_t endian_swap32(uint32_t value);

/// @brief swap the endianess of a 64-bit value
///
/// @param value the value to swap
///
/// @return the swapped value
CT_NODISCARD CT_CONSTFN
CT_ENDIAN_API uint64_t endian_swap64(uint64_t value);

/// @brief convert a 16-bit value of a given endianess to the native endianess
///
/// @param value the value to convert
/// @param order the endianess of @a value
///
/// @return the converted value
CT_NODISCARD CT_CONSTFN
CT_ENDIAN_API uint16_t native_order16(uint16_t value, IN_DOMAIN(<, eEndianCount) endian_t order);

/// @brief convert a 32-bit value of a given endianess to the native endianess
///
/// @param value the value to convert
/// @param order the endianess of @a value
///
/// @return the converted value
CT_NODISCARD CT_CONSTFN
CT_ENDIAN_API uint32_t native_order32(uint32_t value, IN_DOMAIN(<, eEndianCount) endian_t order);

/// @brief convert a 64-bit value of a given endianess to the native endianess
///
/// @param value the value to convert
/// @param order the endianess of @a value
///
/// @return the converted value
CT_NODISCARD CT_CONSTFN
CT_ENDIAN_API uint64_t native_order64(uint64_t value, IN_DOMAIN(<, eEndianCount) endian_t order);

/// @}

CT_END_API
