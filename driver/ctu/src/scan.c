#include "ctu/scan.h"

#include "base/macros.h"
#include "base/util.h"

#include "report/report.h"

#include "scan/node.h"

ctu_digit_t ctu_parse_digit(const char *str, size_t base)
{
    ctu_digit_t result = {
        .value = 0
    };

    mpz_init_set_str(result.value, str, (int)base);

    return result;
}

ctu_string_t ctu_parse_string(reports_t *reports, const char *str, size_t length)
{
    CTU_UNUSED(reports);

    ctu_string_t result = {
        .text = ctu_strdup(str),
        .length = length
    };

    return result;
}

void ctuerror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    CTU_UNUSED(state);

    report(scan_reports(scan), eFatal, node_new(scan, *where), "%s", msg);
}
