#pragma once

typedef struct hlir_t hlir_t;

typedef enum hlir_kind_t
{
#define HLIR_KIND(ID, _) ID,
#include "hlir-def.inc"

    eHlirTotal
} hlir_kind_t;

typedef struct hlir_t {
    hlir_kind_t kind;
} hlir_t;
