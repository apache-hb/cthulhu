#include "json/scan.h"

json_scan_t *json_scan_context(scan_t *scan)
{
    return scan_get_context(scan);
}
