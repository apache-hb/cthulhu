// SPDX-License-Identifier: GPL-3.0-only

#include "cthulhu/broker/scan.h"

void pl0error(where_t *where, void *state, scan_t *scan, const char *msg)
{
    ctx_error(where, state, scan, msg);
}
