#include "oberon/scan.h"

#include "report/report.h"

#include "base/macros.h"
#include "base/util.h"

obr_string_t obr_parse_string(scan_t *scan, const char *str, size_t length)
{
    CTU_UNUSED(scan);

    obr_string_t result = {
        .text = ctu_strndup(str, length),
        .length = length
    };

    return result;
}

void obrerror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    CTU_UNUSED(state);

    report(scan_reports(scan), eFatal, node_new(scan, *where), "%s", msg);
}
