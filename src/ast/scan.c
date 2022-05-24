#include "cthulhu/ast/scan.h"

#include "cthulhu/report/report.h"

size_t scan_size(const scan_t *scan)
{
    return scan->source.size;
}

const char *scan_text(const scan_t *scan)
{
    return scan->source.text;
}

void scan_set(scan_t *scan, void *data)
{
    logverbose("[scan-set] %p", data);
    scan->data = data;
}

void *scan_get(scan_t *scan)
{
    logverbose("[scan-get] %p", scan->data);
    return scan->data;
}
