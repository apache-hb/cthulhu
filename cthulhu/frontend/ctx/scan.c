#include "scan.h"

#include "cthulhu/util/report.h"

#include "ctx-bison.h"
#include "ctx-flex.h"

static int init(scan_t *extra, void *scanner) {
    return ctxlex_init_extra(extra, scanner);
}

static void set_in(FILE *fd, void *scanner) {
    ctxset_in(fd, scanner);
}

static int parse(scan_t *extra, void *scanner) {
    return ctxparse(scanner, extra);
}

static void *scan(const char *text, void *scanner) {
    return ctx_scan_string(text, scanner);
}

static void destroy(void *scanner) {
    ctxlex_destroy(scanner);
}

static callbacks_t CALLBACKS = {
    .init = init,
    .set_in = set_in,
    .parse = parse,
    .scan = scan,
    .destroy = destroy
};

ctx_t *ctx_compile(scan_t *scan) {
    return compile_file(scan, &CALLBACKS);
}

scan_t ctx_open(reports_t *reports, file_t *file) {
    return scan_file(reports, "cthulhu-x", file);
}

void ctxerror(where_t *where, void *state, scan_t *scan, const char *msg) {
    UNUSED(state);

    report(scan->reports, ERROR, node_new(scan, *where), "%s", msg);
}
