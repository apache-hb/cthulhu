#include "ctu/scan.h"

#include "base/macros.h"
#include "base/util.h"

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

    report(scan_reports(scan), eFatal, node_new(scan, *where), "%s", msg);
}
