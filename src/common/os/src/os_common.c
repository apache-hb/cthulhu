#include "base/panic.h"
#include "os/core.h"

static const char *const kDirentNames[eOsNodeCount] = {
#define OS_DIRENT(ID, STR) [ID] = (STR),
#include "os/os.def"
};

USE_DECL
const char *os_dirent_string(os_dirent_t type)
{
    CTASSERTF(type < eOsNodeCount, "invalid dirent type %d", type);

    return kDirentNames[type];
}
