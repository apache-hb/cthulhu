// SPDX-License-Identifier: LGPL-3.0-only

#include "scan/scan.h"

#include "core/text.h"
#include "base/panic.h"
#include "base/util.h"

#include "arena/arena.h"
#include "io/io.h"

static scan_t *scan_new(const char *language, const char *path, io_t *io, arena_t *arena)
{
    CTASSERT(language != NULL);
    CTASSERT(path != NULL);
    CTASSERT(arena != NULL);

    scan_t *self = ARENA_MALLOC(sizeof(scan_t), path, NULL, arena);
    self->language = language;
    self->path = path;
    self->arena = arena;
    self->nodes = arena;

    self->tree = NULL;
    self->context = NULL;

    self->io = io;

    return self;
}

USE_DECL
scan_t *scan_builtin(const char *language, arena_t *arena)
{
    scan_t *scan = scan_new(language, CT_SCAN_BUILTIN_NAME, NULL, arena);
    scan->mapped = text_view_from("");
    return scan;
}

USE_DECL
bool scan_is_builtin(const scan_t *scan)
{
    CTASSERT(scan != NULL);
    return scan->io == NULL;
}

USE_DECL
const char *scan_language(const scan_t *scan)
{
    CTASSERT(scan != NULL);

    return scan->language;
}

USE_DECL
const char *scan_path(const scan_t *scan)
{
    CTASSERT(scan != NULL);

    return scan->path;
}

USE_DECL
void scan_set(scan_t *scan, void *value)
{
    CTASSERT(scan != NULL);

    scan->tree = value;
}

USE_DECL
void *scan_get(scan_t *scan)
{
    CTASSERT(scan != NULL);

    return scan->tree;
}

USE_DECL
void scan_set_context(scan_t *scan, void *value)
{
    CTASSERT(scan != NULL);

    scan->context = value;
}

USE_DECL
void *scan_get_context(const scan_t *scan)
{
    CTASSERT(scan != NULL);

    return scan->context;
}

USE_DECL
text_view_t scan_source(const scan_t *scan)
{
    CTASSERT(scan != NULL);

    return scan->mapped;
}

USE_DECL CT_NOALIAS
size_t scan_read(scan_t *scan, void *dst, size_t size)
{
    CTASSERT(scan != NULL);
    CTASSERT(dst != NULL);

    return io_read(scan->io, dst, size);
}

USE_DECL
arena_t *scan_get_arena(const scan_t *scan)
{
    CTASSERT(scan != NULL);

    return scan->arena;
}

USE_DECL
scan_t *scan_io(const char *language, io_t *io, arena_t *arena)
{
    CTASSERT(io != NULL);

    os_error_t err = io_error(io);
    const char *path = io_name(io);

    CTASSERT(arena != NULL);
    CTASSERTF(err == 0, "constructing scanner from an io object (%s) thats in an error state: %s", path, os_error_string(err, arena));

    const void *region = io_map(io, eOsProtectRead);
    size_t size = io_size(io);

    CTASSERTF(region != NULL, "failed to map %s of size %zu (%s)", path, size, os_error_string(io_error(io), arena));

    scan_t *self = scan_new(language, path, io, arena);

    self->mapped = text_view_make(region, size);

    return self;
}
