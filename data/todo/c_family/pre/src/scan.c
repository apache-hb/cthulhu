// SPDX-License-Identifier: GPL-3.0-only

#include "pre/scan.h"

cpp_scan_t *cpp_scan_context(scan_t *scan)
{
    return scan_get_context(scan);
}
