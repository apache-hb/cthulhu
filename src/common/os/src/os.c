// SPDX-License-Identifier: LGPL-3.0-only

#include "os/core.h"
#include "os/os.h"
#include "os_common.h"

#include "arena/arena.h"
#include "base/panic.h"
#include "core/text.h"

USE_DECL
char *os_error_string(os_error_t error, arena_t *arena)
{
    CTASSERT(arena != NULL);

    size_t size = os_error_get_string(error, NULL, 0);
    CTASSERT(size > 0);

    char *buffer = ARENA_MALLOC(size + 1, "os_error_string", NULL, arena);

    os_error_get_string(error, buffer, size + 1);

    return buffer;
}

USE_DECL
os_error_t os_getcwd(text_t *cwd, arena_t *arena)
{
    CTASSERT(cwd != NULL);
    CTASSERT(arena != NULL);

    size_t size = os_cwd_get_string(NULL, 0);
    CTASSERT(size > 0);

    char *buffer = ARENA_MALLOC(size + 1, "os_getcwd", NULL, arena);

    size_t ret = os_cwd_get_string(buffer, size + 1);
    if (ret == 0)
    {
        return os_last_error();
    }

    cwd->text = buffer;
    cwd->length = ret;

    return eOsSuccess;
}

USE_DECL
char *os_dir_string(const os_inode_t *dir, arena_t *arena)
{
    CTASSERT(dir != NULL);
    CTASSERT(arena != NULL);

    size_t size = os_dir_get_string(dir, NULL, 0);
    CTASSERT(size > 0);

    char *buffer = ARENA_MALLOC(size + 1, "os_dir_name", NULL, arena);

    os_dir_get_string(dir, buffer, size + 1);

    return buffer;
}
