#include "pre/scan.h"

cpp_scan_t *cpp_scan_context(scan_t *scan)
{
    return scan_get_context(scan);
}
