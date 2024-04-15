#pragma once

#include <stddef.h>

/// @ingroup core
/// @{

/// @brief the container length type
/// seperately defined as some compilers have support
/// for unsigned types with undefined overflow behaviour.
/// we can use these to get more effective ubsan checks.
/// @note i think it can also help with optimization but i'm not sure
typedef size_t ctu_length_t;
#define CTU_LENGTH_MAX SIZE_MAX

typedef size_t ctu_hash_t;
#define CTU_HASH_MAX SIZE_MAX

typedef unsigned char ctu_byte_t;
#define CTU_BYTE_MAX UCHAR_MAX

/// @}
