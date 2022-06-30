#pragma once

/**
 * @brief the width of an integer type
 */
typedef enum
{
    eChar,  ///< a range of at least -127 to 127 if signed or 0 to 255 if
                 ///< unsigned
    eShort, ///< a range of at least -32767 to 32767 if signed or 0 to
                 ///< 65535 if unsigned
    eInt,   ///< a range of at least -2147483647 to 2147483647 if signed or 0
                 ///< to 4294967295 if unsigned
    eLong,  ///< a range of at least -9223372036854775807 to
                 ///< 9223372036854775807 if signed or 0 to 18446744073709551615
                 ///< if unsigned

    eIntSize, ///< the size of any type
    eIntPtr,  ///< the same width as a pointer
    eIntMax,  ///< the largest native platform integer

    eDigitTotal
} digit_t;

/**
 * @brief the sign of an integer type
 */
typedef enum
{
    eSigned,   ///< a signed type
    eUnsigned, ///< an unsigned type

    eSignTotal
} sign_t;
