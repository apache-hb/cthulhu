#include "scan.h"

#include "cthulhu/util/report.h"

#include <ctype.h>

char *pl0_normalize(const char *ident) {
    char *out = ctu_strdup(ident);
    for (char *p = out; *p; ++p) {
        *p = tolower(*p);
    }
    return out;
}

void pl0error(where_t *where, void *state, scan_t *scan, const char *msg) {
    UNUSED(state);

    report(scan->reports, ERROR, node_new(scan, *where), "%s", msg);
}
