#pragma once

#include <stdint.h>

#include "jvm/common.h"

typedef struct jvm_classfile_t {
    jvm_version_t majorVersion;
    uint16_t minorVersion;

    jvm_access_t accessFlags;
} jvm_classfile_t;
