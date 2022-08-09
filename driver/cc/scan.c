#include "scan.h"
#include "sema.h"

void init_scan(scan_t *scan)
{
    size_t sizes[eTagTotal] = {
        [eSemaTypes] = 32,
        [eSemaValues] = 32,
        [eSemaProcs] = 32,
        [eSemaModules] = 32
    };

    sema_t *sema = sema_new(NULL, scan_reports(scan), eTagTotal, sizes);
    scan_set(scan, sema);
}

bool cc_is_typename(scan_t *scan, const char *text)
{
    return sema_get(scan_get(scan), eSemaTypes, text);
}
