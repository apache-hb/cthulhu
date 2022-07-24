#pragma once

typedef enum {
#define DIGIT_KIND(ID, STR) ID,
#include "hlir-def.inc"

    eDigitTotal
} digit_t;

typedef enum {
#define SIGN_KIND(ID, STR) ID,
#include "hlir-def.inc"

    eSignTotal
} sign_t;
