#include "target/target.h"

#include "base/panic.h"

static const char *const kFileLayoutNames[] = {
#define FILE_LAYOUT(ID, STR) [ID] = (STR),
#include "target/target.inc"
};

USE_DECL
const char *file_layout_str(file_layout_t layout)
{
    CTASSERTF(layout < eFileLayoutCount, "invalid file layout %d", layout);
    return kFileLayoutNames[layout];
}
