#include "common.h"

#include "report/report.h"

#include "base/macros.h"
#include "base/memory.h"
#include "base/panic.h"

#include "io/io.h"

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
text_t scan_source(const scan_t *scan)
{
    CTASSERT(scan != NULL);
    
    text_t text = {scan->size, scan->mapped};
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
    
    return io_read(scan->io, dst, size);
}

USE_DECL
reports_t *scan_reports(scan_t *scan)
{
    CTASSERT(scan != NULL);
    
    return scan->reports;
}

USE_DECL
scan_t *scan_invalid(void)
{
    return NULL;
}

USE_DECL
scan_t *scan_io(reports_t *reports, const char *language, io_t *io)
{
    CTASSERT(reports != NULL);
    CTASSERT(language != NULL);
    CTASSERT(io != NULL);
    CTASSERTF(io_error(io) == 0, "io-error(%s) = %zu", io_name(io), io_error(io));

    scan_t *self = ctu_malloc(sizeof(scan_t));

    self->language = language;
    self->reports = reports;
    self->io = io;

    self->mapped = io_map(io);
    self->size = io_size(io);

    return self;
}
