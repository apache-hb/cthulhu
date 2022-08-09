#pragma once

#include "cthulhu/hlir/sema.h"

typedef enum {
    eTagStructs = eSemaMax,
    eTagUnions,
    eTagEnums,

    eTagTotal
} tag_t;
