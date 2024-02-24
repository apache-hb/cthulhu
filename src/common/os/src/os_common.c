#include "os/core.h"

#include "base/panic.h"

static const char *const kDirentNames[eOsNodeCount] = {
#define OS_DIRENT(ID, STR) [ID] = (STR),
#include "os/os.inc"
};

USE_DECL
const char *os_dirent_string(os_dirent_t type)
{
    CTASSERTF(type < eOsNodeCount, "invalid dirent type %d", type);

    return kDirentNames[type];
}
