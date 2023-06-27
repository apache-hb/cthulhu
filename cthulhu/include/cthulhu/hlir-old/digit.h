#pragma once

/**
 * @brief the width of an integer type
 */
typedef enum
{
#define DIGIT_KIND(ID, STR) ID,
#include "hlir-def.inc"
    eDigitTotal
} digit_t;

/**
 * @brief the sign of an integer type
 */
typedef enum
{
#define SIGN_KIND(ID, STR) ID,
#include "hlir-def.inc"
    eSignTotal
} sign_t;
