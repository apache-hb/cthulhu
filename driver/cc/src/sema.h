#pragma once

#include "cthulhu/tree/sema.h"

typedef enum {
    eTagStructs = eSemaMax,
    eTagUnions,
    eTagEnums,

    eTagTotal
} tag_t;
