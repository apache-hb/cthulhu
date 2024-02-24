// SPDX-License-Identifier: GPL-3.0-only

#include "ctu/scan.h"

#include "scan/node.h"

#include "cthulhu/broker/scan.h"
#include "cthulhu/events/events.h"

void ctu_parse_digit(scan_t *scan, where_t where, mpz_t digit, const char *str, size_t base)
{
    int ret = mpz_init_set_str(digit, str, (int)base);

    if (ret == -1)
    {
        const node_t *node = node_new(scan, where);
        logger_t *logger = ctx_get_logger(scan);
        msg_notify(logger, &kEvent_InvalidIntegerLiteral, node, "failed to parse base %zu digit '%s'", base, str);
    }
}

void ctuerror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    ctx_error(where, state, scan, msg);
}
