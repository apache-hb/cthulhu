#include "scan.h"

void ctuerror(where_t *where, void *state, scan_t *scan, const char *msg) {
    UNUSED(state);

    report(scan->reports, ERROR, node_new(scan, *where), "%s", msg);
}

void enter_template(scan_t *scan) {
    lex_extra_t *extra = scan_get(scan);
    extra->depth++;
}

size_t exit_template(scan_t *scan) {
    lex_extra_t *extra = scan_get(scan);
    return extra->depth--;
}
