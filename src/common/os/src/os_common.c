// SPDX-License-Identifier: LGPL-3.0-only

#include "os/os.h" // IWYU pragma: keep

#include "base/panic.h"

static const char *const kDirentNames[eOsNodeCount] = {
#define OS_DIRENT(ID, STR) [ID] = (STR),
#include "os/os.inc"
};

STA_DECL
const char *os_dirent_string(os_dirent_t type)
{
    CT_ASSERT_RANGE(type, 0, eOsNodeCount - 1);

    return kDirentNames[type];
}

static const char *const kAccessNames[] = {
    [eOsAccessNone] = "none",
    [eOsAccessRead] = "read",
    [eOsAccessWrite] = "write",
    [eOsAccessRead | eOsAccessWrite] = "read/write",
    [eOsAccessTruncate] = "truncate",
    [eOsAccessRead | eOsAccessTruncate] = "read (truncate)",
    [eOsAccessWrite | eOsAccessTruncate] = "write (truncate)",
    [eOsAccessRead | eOsAccessWrite | eOsAccessTruncate] = "read/write (truncate)",
};

STA_DECL
const char *os_access_string(os_access_t access)
{
    CTASSERTF(!(access & ~eOsAccessMask), "invalid access flags 0x%x", access);
    return kAccessNames[access];
}

static const char *const kProtectNames[] = {
    [eOsProtectNone] = "none",
    [eOsProtectRead] = "read",
    [eOsProtectWrite] = "write",
    [eOsProtectExecute] = "execute",
    [eOsProtectRead | eOsProtectWrite] = "read/write",
    [eOsProtectRead | eOsProtectExecute] = "read/execute",
    [eOsProtectWrite | eOsProtectExecute] = "write/execute",
    [eOsProtectRead | eOsProtectWrite | eOsProtectExecute] = "read/write/execute",
};

STA_DECL
const char *os_protect_string(os_protect_t protect)
{
    CTASSERTF(!(protect & ~eOsProtectMask), "invalid protect flags 0x%x", protect);
    return kProtectNames[protect];
}
