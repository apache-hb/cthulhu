#pragma once

typedef enum {
#define HLIR_LINKAGE(ID, STR) ID,
#include "hlir-def.inc"
} HlirLinkage;

typedef enum {
#define HLIR_VISIBILITY(ID, NAME) ID,
#include "hlir-def.inc"
} HlirVisibility;

typedef enum {
#define TYPE_QUALIFIER(ID, STR, BIT) ID = (BIT),
#include "hlir-def.inc"
} HlirQualifiers;

typedef struct {
    HlirLinkage linkage;
    HlirVisibility visibility;
    HlirQualifiers qualifiers;
    
    const char *mangle;
} HlirAttribs;
