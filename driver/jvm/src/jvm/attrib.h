#pragma once

#include <stdint.h>

typedef struct jvm_attrib_t {
    uint16_t nameIndex;
    uint32_t length;
    uint8_t *info;
} jvm_attrib_t;
