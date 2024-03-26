// SPDX-License-Identifier: LGPL-3.0-only

#include "os/os.h" // IWYU pragma: keep

#include "base/panic.h"

#if CTU_EVENTS
os_events_t gOsEvents = { 0 };
#endif

static const char *const kDirentNames[eOsNodeCount] = {
#define OS_DIRENT(ID, STR) [ID] = (STR),
#include "os/os.inc"
};

USE_DECL
const char *os_dirent_string(os_dirent_t type)
{
    CT_ASSERT_RANGE(type, 0, eOsNodeCount - 1);

    return kDirentNames[type];
}

USE_DECL
const char *os_access_string(os_access_t access)
{
    switch (access)
    {
    case (eOsAccessRead | eOsAccessWrite | eOsAccessTruncate):
        return "read/write (truncate)";
    case (eOsAccessWrite | eOsAccessTruncate):
        return "write (truncate)";
    case (eOsAccessRead | eOsAccessTruncate):
        return "read (truncate)";

    case (eOsAccessRead | eOsAccessWrite):
        return "read/write";
    case eOsAccessRead:
        return "read";
    case eOsAccessWrite:
        return "write";

    case eOsAccessNone:
        return "none";

    default:
        CT_NEVER("invalid access flags 0x%x", access);
    }
}

USE_DECL
const char *os_protect_string(os_protect_t protect)
{
    switch (protect)
    {
    case (eOsProtectRead | eOsProtectWrite | eOsProtectExecute):
        return "read/write/execute";
    case (eOsProtectRead | eOsProtectExecute):
        return "read/execute";
    case (eOsProtectWrite | eOsProtectExecute):
        return "write/execute";
    case (eOsProtectRead | eOsProtectWrite):
        return "read/write";

    case eOsProtectRead:
        return "read";
    case eOsProtectWrite:
        return "write";
    case eOsProtectExecute:
        return "execute";

    case eOsProtectNone:
        return "none";

    default:
        CT_NEVER("invalid protect flags 0x%x", protect);
    }
}
