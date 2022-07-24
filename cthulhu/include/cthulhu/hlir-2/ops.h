#pragma once

typedef enum {
#define UNARY_OP(ID, NAME, SYMBOL) ID,
#include "hlir-def.inc"

    eUnaryTotal
} unary_t;

typedef enum {
#define BINARY_OP(ID, NAME, SYMBOL) ID,
#include "hlir-def.inc"

    eBinaryTotal
} binary_t;

typedef enum {
#define COMPARE_OP(ID, NAME, SYMBOL) ID,
#include "hlir-def.inc"

    eCompareTotal
} compare_t;

typedef enum {
#define HLIR_BUILTIN(ID, STR) ID,
#include "hlir-def.inc"

    eBuiltinTotal
} builtin_t;
