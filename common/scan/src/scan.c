#include "common.h"

#include "base/panic.h"

#include "io/io.h"
#include "memory/arena.h"

#include <limits.h>
#include <string.h>

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

    return io_name(scan->io);
}

USE_DECL
void *scan_get(scan_t *scan)
{
    CTASSERT(scan != NULL);

    return scan->data;
}

USE_DECL
void scan_set(scan_t *scan, void *value)
{
    CTASSERT(scan != NULL);

    scan->data = value;
}

USE_DECL
const char *scan_text(const scan_t *scan)
{
    CTASSERT(scan != NULL);

    return scan->mapped;
}

USE_DECL
text_view_t scan_source(const scan_t *scan)
{
    CTASSERT(scan != NULL);

    text_view_t text = {
        .text = scan->mapped,
        .size = scan->size
    };

    return text;
}

USE_DECL
size_t scan_size(const scan_t *scan)
{
    CTASSERT(scan != NULL);

    return io_size(scan->io);
}

USE_DECL
size_t scan_read(scan_t *scan, void *dst, size_t size)
{
    CTASSERT(scan != NULL);
    CTASSERT(dst != NULL);

    return io_read(scan->io, dst, size);
}

USE_DECL
reports_t *scan_reports(scan_t *scan)
{
    CTASSERT(scan != NULL);

    return scan->reports;
}

USE_DECL
arena_t *scan_alloc(scan_t *scan)
{
    CTASSERT(scan != NULL);

    return scan->alloc;
}

USE_DECL
io_t *scan_src(scan_t *scan)
{
    CTASSERT(scan != NULL);

    return scan->io;
}

USE_DECL
scan_t *scan_invalid(void)
{
    return NULL;
}

USE_DECL
scan_t *scan_io(reports_t *reports, const char *language, io_t *io, arena_t *alloc)
{
    CTASSERT(reports != NULL);
    CTASSERT(language != NULL);
    CTASSERT(io != NULL);
    CTASSERTF(io_error(io) == 0, "io-error(%s) = %zu", io_name(io), io_error(io));
    CTASSERT(alloc != NULL);

    scan_t *self = ARENA_MALLOC(alloc, sizeof(scan_t), io_name(io), NULL);

    self->language = language;
    self->reports = reports;
    self->io = io;
    self->alloc = alloc;

    self->mapped = io_map(io);
    self->size = io_size(io);

    return self;
}
