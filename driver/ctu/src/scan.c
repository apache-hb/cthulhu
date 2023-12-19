#include "ctu/scan.h"

#include "core/macros.h"

#include "report/report.h"

#include "scan/node.h"

ctu_digit_t ctu_parse_digit(scan_t *scan, const char *str, size_t base)
{
    CTU_UNUSED(scan);

    ctu_digit_t result = {
        .value = 0
    };

    mpz_init_set_str(result.value, str, (int)base);

    return result;
}

void ctuerror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    CTU_UNUSED(state);

    reports_t *reports = scan_get_context(scan);

    report(reports, eFatal, node_new(scan, *where), "%s", msg);
}
