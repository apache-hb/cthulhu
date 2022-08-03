#include "common.h"

#include "platform/file.h"
#include "report/report.h"

#include "base/macros.h"
#include "base/memory.h"
#include "base/panic.h"

#include "io/io.h"

#include <limits.h>
#include <string.h>

const char *scan_language(const scan_t *scan)
{
    return scan->language;
}

const char *scan_path(const scan_t *scan)
{
    return io_name(scan->io);
}

void *scan_get(scan_t *scan)
{
    return scan->data;
}

void scan_set(scan_t *scan, void *value)
{
    scan->data = value;
}

const char *scan_text(const scan_t *scan)
{
    return scan->mapped;
}

text_t scan_source(const scan_t *scan)
{
    text_t text = {scan->size, scan->mapped};
    return text;
}

size_t scan_size(const scan_t *scan)
{
    return scan->size;
}

size_t scan_read(scan_t *scan, void *dst, size_t size)
{
    return io_read(scan->io, dst, size);
}

reports_t *scan_reports(scan_t *scan)
{
    return scan->reports;
}

scan_t *scan_invalid(void)
{
    return NULL;
}

scan_t *scan_io(reports_t *reports, const char *language, io_t *io)
{
    scan_t *self = ctu_malloc(sizeof(scan_t));

    self->language = language;
    self->reports = reports;
    self->io = io;

    self->mapped = io_map(io);
    self->size = io_size(io);

    return self;
}
