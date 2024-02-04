#include "arena/arena.h"
#include "os/core.h"

#include "base/panic.h"
#include "base/util.h"

#include <stdlib.h>
#include <errno.h>

void os_init(void)
{
    // empty
}

USE_DECL
os_error_t os_path_parse(const char *path, arena_t *arena, text_t *out)
{
    CTASSERT(path != NULL);
    CTASSERT(arena != NULL);
    CTASSERT(out != NULL);

    // get the absolute path to the file
    char *abs = realpath(path, NULL);

    if (abs == NULL)
    {
        return errno;
    }

    char *copy = arena_strdup(abs, arena);

    // allocate a new path
    text_t p = text_from(copy);

    // free the absolute path
    // very annoying theres no way to get the realpath without mallocing
    free(abs);

    *out = p;
    return 0;
}

USE_DECL
char *os_path_string(const text_t *path, arena_t *arena)
{
    CTASSERT(path != NULL);
    CTASSERT(arena != NULL);

    char *str = ARENA_MALLOC(path->length + 1, "path", NULL, arena);

    // copy into str, replacing null terminators with forward slashes
    for (size_t i = 0; i < path->length; i++)
    {
        if (path->text[i] == '\0')
        {
            str[i] = '/';
        }
        else
        {
            str[i] = path->text[i];
        }
    }

    str[path->length] = '\0';

    return str;
}
