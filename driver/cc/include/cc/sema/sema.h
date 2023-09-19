#pragma once

#include "cthulhu/tree/sema.h"

typedef enum cc_tag_t {
    eCTagValues = eSemaValues,
    eCTagTypes = eSemaTypes,
    eCTagProcs = eSemaProcs,
    eCTagModules = eSemaModules,

    eCTagTotal
} cc_tag_t;
