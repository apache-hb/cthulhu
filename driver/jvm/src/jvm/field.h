#pragma once

#include "common.h"
#include "attrib.h"

#include <stdint.h>

typedef struct jvm_field_t {
    jvm_access_t accessFlags;
    uint16_t nameIndex;
    uint16_t descriptorIndex;
    uint16_t attributesCount;
    jvm_attrib_t *attributes;
} jvm_field_t;
