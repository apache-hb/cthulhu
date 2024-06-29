// SPDX-License-Identifier: GPL-3.0-only

#include "ctu/scan.h"
#include "arena/arena.h"
#include "ctu/ast.h"

#include "scan/node.h"

#include "cthulhu/broker/scan.h"
#include "cthulhu/events/events.h"
#include "std/str.h"

#include <gmp.h>

void ctu_parse_digit(scan_t *scan, where_t where, ctu_integer_t *integer, const char *str, int len, size_t base)
{
    // TODO: this chain could be better
    if (str_endswithn(str, len, "ul"))
    {
        len -= 2;
        integer->digit = eDigitLong;
        integer->sign = eSignUnsigned;
    }
    else if (str_endswithn(str, len, "uz"))
    {
        len -= 2;
        integer->digit = eDigitSize;
        integer->sign = eSignUnsigned;
    }
    else if (str_endswithn(str, len, "l"))
    {
        len--;
        integer->digit = eDigitLong;
        integer->sign = eSignSigned;
    }
    else if (str_endswithn(str, len, "u"))
    {
        len--;
        integer->digit = eDigitInt;
        integer->sign = eSignUnsigned;
    }
    else
    {
        integer->digit = eDigitInt;
        integer->sign = eSignSigned;
    }

    // TODO: i should maybe submit a patch to gmp to support a length argument
    arena_t *arena = ctx_get_arena(scan);
    char *copy = arena_strndup(str, len, arena);
    int ret = mpz_init_set_str(integer->value, copy, (int)base);
    arena_free(copy, len + 1, arena); // TODO: this +1 is a bit weird

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
