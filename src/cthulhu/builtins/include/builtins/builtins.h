#pragma once

#include <ctu_builtins_api.h>

#include "cthulhu/tree/tree.h"

CT_BEGIN_API

typedef enum attrib_link_t
{
#define ATTRIB_LINK(id, str) id,
#include "builtins/builtins.def"

    eLinkAttribCount
} attrib_link_t;

CT_END_API
