// SPDX-License-Identifier: GPL-3.0-only

#include "target/target.h"

#include "base/panic.h"

static const char *const kFileLayoutNames[] = {
#define FILE_LAYOUT(ID, STR) [ID] = (STR),
#include "target/target.inc"
};

STA_DECL
const char *file_layout_str(file_layout_t layout)
{
    CTASSERTF(layout < eFileLayoutCount, "invalid file layout %d", layout);
    return kFileLayoutNames[layout];
}
