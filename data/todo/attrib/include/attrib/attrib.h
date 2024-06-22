#pragma once

#include <ctu_attrib_api.h>

CT_BEGIN_API

typedef enum attrib_link_t
{
#define ATTRIB_LINK(id, str) id,
#include "attrib/attrib.def"

    eLinkAttribCount
} attrib_link_t;

CT_END_API
