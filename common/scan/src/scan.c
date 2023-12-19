#include "base/panic.h"

#include "core/text.h"
#include "io/io.h"
#include "memory/arena.h"

#include <limits.h>
#include <string.h>

typedef struct reports_t reports_t;

typedef struct scan_t
{
    reports_t *reports; ///< the reporting sink for this file
    io_t *io;           ///< file itself
    arena_t *alloc;     ///< allocator to use everything involving this file

    const char *language; ///< the language this file contains
    const char *path;     ///< the path to this file
    void *data;           ///< user data pointer

    const char *mapped;
    size_t size;
} scan_t;

static scan_t kBuiltinScan = {
    .reports = NULL,
    .io = NULL,
    .alloc = NULL,

    .language = "builtin",
    .path = "builtin",
    .data = NULL,

    .mapped = "",
    .size = 0
};

USE_DECL
scan_t *scan_builtin(void)
{
    return &kBuiltinScan;
}

USE_DECL
bool scan_is_builtin(const scan_t *scan)
{
    return scan == scan_builtin();
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

    return scan->size;
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
arena_t *scan_alloc(const scan_t *scan)
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
    self->path = io_name(io);
    self->alloc = alloc;

    self->data = NULL;

    self->mapped = io_map(io);
    self->size = io_size(io);

    return self;
}
